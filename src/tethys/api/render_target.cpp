#include <tethys/api/render_target.hpp>
#include <tethys/api/context.hpp>

namespace tethys::api {
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

    ShadowDepth make_shadow_depth_target() {
        ShadowDepth shadow_depth{};

        Image::CreateInfo depth_image_info{}; {
            depth_image_info.format = vk::Format::eD24UnormS8Uint;
            depth_image_info.width = ctx.swapchain.extent.width * 2;
            depth_image_info.height = ctx.swapchain.extent.height * 2;
            depth_image_info.usage_flags = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
            depth_image_info.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
            depth_image_info.tiling = vk::ImageTiling::eOptimal;
            depth_image_info.samples = vk::SampleCountFlagBits::e1;
            depth_image_info.mips = 1;
        }

        shadow_depth.image = api::make_image(depth_image_info);
        shadow_depth.view = api::make_image_view(shadow_depth.image.handle, vk::Format::eD24UnormS8Uint, vk::ImageAspectFlagBits::eDepth, 1);
        api::transition_image_layout(shadow_depth.image.handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, 1);

        return shadow_depth;
    }
} // namespace tethys::api