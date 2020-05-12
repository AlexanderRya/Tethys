#include <tethys/api/render_target.hpp>
#include <tethys/api/context.hpp>

namespace tethys::api {
    Offscreen make_offscreen_target() {
        Offscreen offscreen{};

        Image::CreateInfo color_image_info{}; {
            color_image_info.format = vk::Format::eB8G8R8A8Srgb;
            color_image_info.width = context.swapchain.extent.width;
            color_image_info.height = context.swapchain.extent.height;
            color_image_info.usage_flags = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
            color_image_info.samples = vk::SampleCountFlagBits::e1;
            color_image_info.tiling = vk::ImageTiling::eOptimal;
            color_image_info.aspect = vk::ImageAspectFlagBits::eColor;
            color_image_info.mips = 1;
        }
        offscreen.color = api::make_image(color_image_info);

        Image::CreateInfo depth_image_info{}; {
            depth_image_info.format = vk::Format::eD32SfloatS8Uint;
            depth_image_info.width = context.swapchain.extent.width;
            depth_image_info.height = context.swapchain.extent.height;
            depth_image_info.usage_flags = vk::ImageUsageFlagBits::eDepthStencilAttachment;
            depth_image_info.tiling = vk::ImageTiling::eOptimal;
            depth_image_info.aspect = vk::ImageAspectFlagBits::eDepth;
            depth_image_info.samples = context.device.samples;
            depth_image_info.mips = 1;
        }
        offscreen.depth = api::make_image(depth_image_info);

        Image::CreateInfo msaa_image_info{}; {
            msaa_image_info.format = vk::Format::eB8G8R8A8Srgb;
            msaa_image_info.width = context.swapchain.extent.width;
            msaa_image_info.height = context.swapchain.extent.height;
            msaa_image_info.usage_flags = vk::ImageUsageFlagBits::eColorAttachment;
            msaa_image_info.aspect = vk::ImageAspectFlagBits::eColor;
            msaa_image_info.samples = context.device.samples;
            msaa_image_info.tiling = vk::ImageTiling::eOptimal;
            msaa_image_info.mips = 1;
        }
        offscreen.msaa = api::make_image(msaa_image_info);

        return offscreen;
    }
} // namespace tethys::api