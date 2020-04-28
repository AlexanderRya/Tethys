#include <tethys/api/context.hpp>
#include <tethys/api/swapchain.hpp>
#include <tethys/api/image.hpp>
#include <tethys/window/window.hpp>
#include <tethys/logger.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
     [[nodiscard]] static u32 get_image_count(const vk::SurfaceCapabilitiesKHR& capabilities) {
        auto count = capabilities.minImageCount + 1;

        if (capabilities.maxImageCount > 0 && count > capabilities.maxImageCount) {
            count = capabilities.maxImageCount;
        }

        logger::info("Swapchain details: image count: ", count);

        return count;
    }

    [[nodiscard]] static vk::Extent2D get_extent(const vk::SurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX) {

            return capabilities.currentExtent;
        } else {
            vk::Extent2D extent{ window::width(), window::height() };

            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return extent;
        }
    }

    [[nodiscard]] static vk::SurfaceFormatKHR get_format() {
        auto surface_formats = ctx.device.physical.getSurfaceFormatsKHR(ctx.surface, {}, ctx.dispatcher);

        vk::SurfaceFormatKHR format = surface_formats[0];

        for (const auto& each : surface_formats) {
            if (each.format == vk::Format::eB8G8R8A8Srgb &&
                each.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {

                logger::info("Swapchain details: format: vk::Format::", vk::to_string(each.format));
                logger::info("Swapchain details: color space: vk::ColorSpaceKHR::", vk::to_string(each.colorSpace));
                return each;
            }
        }

        logger::warning("Swapchain details: non-preferred format: vk::Format::", vk::to_string(format.format));
        logger::warning("Swapchain details: non-preferred color space: vk::ColorSpaceKHR::", vk::to_string(format.colorSpace));

        return format;
    }

    [[nodiscard]] static vk::PresentModeKHR get_present_mode() {
        for (const auto& mode : ctx.device.physical.getSurfacePresentModesKHR(ctx.surface, {}, ctx.dispatcher)) {
            if (mode == vk::PresentModeKHR::eImmediate) {
                logger::info("Swapchain details: present mode: vk::PresentModeKHR::", vk::to_string(mode));
                return mode;
            }
        }

        logger::warning("Swapchain details: non-preferreds present mode: vk::PresentModeKHR::", vk::to_string(vk::PresentModeKHR::eFifo));

        return vk::PresentModeKHR::eFifo;
    }

    static void get_swapchain(Swapchain& swapchain) {
        vk::SwapchainCreateInfoKHR swapchain_create_info{}; {
            swapchain_create_info.surface = ctx.surface;
            swapchain_create_info.minImageCount = swapchain.image_count;
            swapchain_create_info.imageFormat = swapchain.format.format;
            swapchain_create_info.imageColorSpace = swapchain.format.colorSpace;
            swapchain_create_info.imageExtent = swapchain.extent;
            swapchain_create_info.preTransform = swapchain.surface_transform;
            swapchain_create_info.imageArrayLayers = 1;
            swapchain_create_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
            swapchain_create_info.imageSharingMode = vk::SharingMode::eExclusive;
            swapchain_create_info.queueFamilyIndexCount = 1;
            swapchain_create_info.pQueueFamilyIndices = &ctx.device.queue_family;
            swapchain_create_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
            swapchain_create_info.presentMode = swapchain.present_mode;
            swapchain_create_info.clipped = true;
            swapchain_create_info.oldSwapchain = nullptr;
        }

        swapchain.handle = ctx.device.logical.createSwapchainKHR(swapchain_create_info, nullptr, ctx.dispatcher);

        logger::info("Swapchain successfully created");
    }

    static void create_images(Swapchain& swapchain) {
        swapchain.images = ctx.device.logical.getSwapchainImagesKHR(swapchain.handle, ctx.dispatcher);

        swapchain.image_views.reserve(swapchain.image_count);

        for (const auto& image : swapchain.images) {
            swapchain.image_views.emplace_back(api::make_image_view(image, swapchain.format.format, vk::ImageAspectFlagBits::eColor, 1));
        }

        logger::info("Swapchain images successfully created");
    }

    Swapchain make_swapchain() {
        auto capabilities = ctx.device.physical.getSurfaceCapabilitiesKHR(ctx.surface, ctx.dispatcher);

        Swapchain swapchain{};

        swapchain.image_count = get_image_count(capabilities);
        swapchain.extent = get_extent(capabilities);
        swapchain.format = get_format();
        swapchain.present_mode = get_present_mode();

        swapchain.surface_transform = capabilities.currentTransform;

        get_swapchain(swapchain);
        create_images(swapchain);

        return swapchain;
    }

    Offscreen make_offscreen_target() {
        Offscreen offscreen{};

        Image::CreateInfo color_image_info{}; {
            color_image_info.format = vk::Format::eB8G8R8A8Srgb;
            color_image_info.width = ctx.swapchain.extent.width;
            color_image_info.height = ctx.swapchain.extent.height;
            color_image_info.usage_flags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
            color_image_info.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
            color_image_info.samples = vk::SampleCountFlagBits::e1;
            color_image_info.tiling = vk::ImageTiling::eOptimal;
            color_image_info.mips = 1;
        }

        offscreen.image = api::make_image(color_image_info);

        offscreen.image_view = api::make_image_view(offscreen.image.handle, vk::Format::eB8G8R8A8Srgb, vk::ImageAspectFlagBits::eColor, 1);

        Image::CreateInfo depth_image_info{}; {
            depth_image_info.format = vk::Format::eD24UnormS8Uint;
            depth_image_info.width = ctx.swapchain.extent.width;
            depth_image_info.height = ctx.swapchain.extent.height;
            depth_image_info.usage_flags = vk::ImageUsageFlagBits::eDepthStencilAttachment;
            depth_image_info.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
            depth_image_info.tiling = vk::ImageTiling::eOptimal;
            depth_image_info.samples = ctx.device.max_samples;
            depth_image_info.mips = 1;
        }

        offscreen.depth_image = api::make_image(depth_image_info);
        offscreen.depth_view = api::make_image_view(offscreen.depth_image.handle, vk::Format::eD24UnormS8Uint, vk::ImageAspectFlagBits::eDepth, 1);
        api::transition_image_layout(offscreen.depth_image.handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);

        Image::CreateInfo msaa_image_info{}; {
            msaa_image_info.format = vk::Format::eB8G8R8A8Srgb;
            msaa_image_info.width = ctx.swapchain.extent.width;
            msaa_image_info.height = ctx.swapchain.extent.height;
            msaa_image_info.usage_flags = vk::ImageUsageFlagBits::eColorAttachment;
            msaa_image_info.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
            msaa_image_info.samples = ctx.device.max_samples;
            msaa_image_info.tiling = vk::ImageTiling::eOptimal;
            msaa_image_info.mips = 1;
        }

        offscreen.msaa_image = api::make_image(msaa_image_info);
        offscreen.msaa_view = api::make_image_view(offscreen.msaa_image.handle, vk::Format::eB8G8R8A8Srgb, vk::ImageAspectFlagBits::eColor, 1);

        return offscreen;
    }
} // namespace tethys::api