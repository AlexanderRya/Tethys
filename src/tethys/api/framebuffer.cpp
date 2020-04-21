#include <tethys/api/framebuffer.hpp>
#include <tethys/api/context.hpp>
#include <tethys/logger.hpp>

namespace tethys::api {
    std::vector<vk::Framebuffer> make_default_framebuffers() {
        std::vector<vk::Framebuffer> framebuffers{};
        framebuffers.reserve(ctx.swapchain.image_count);

        vk::FramebufferCreateInfo framebuffer_create_info{}; {
            framebuffer_create_info.renderPass = ctx.default_render_pass;
            framebuffer_create_info.height = ctx.swapchain.extent.height;
            framebuffer_create_info.width = ctx.swapchain.extent.width;
            framebuffer_create_info.layers = 1;
        }

        std::array<vk::ImageView, 2> attachments{};

        for (const auto& image_view : ctx.swapchain.image_views) {
            attachments[0] = image_view;
            attachments[1] = ctx.swapchain.depth_view;

            framebuffer_create_info.attachmentCount = attachments.size();
            framebuffer_create_info.pAttachments = attachments.data();

            framebuffers.emplace_back(ctx.device.logical.createFramebuffer(framebuffer_create_info, nullptr, ctx.dispatcher));
        }

        logger::info("Framebuffers successfully created with default renderpass");

        return framebuffers;
    }
} // namespace tethys::api