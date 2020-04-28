#include <tethys/api/framebuffer.hpp>
#include <tethys/api/context.hpp>
#include <tethys/logger.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    vk::Framebuffer make_offscreen_framebuffer(const vk::RenderPass offscreen_render_pass) {
        vk::Framebuffer framebuffer{};

        std::array<vk::ImageView, 3> attachments{}; {
            attachments[0] = ctx.offscreen.msaa_view;
            attachments[1] = ctx.offscreen.depth_view;
            attachments[2] = ctx.offscreen.image_view;
        }

        vk::FramebufferCreateInfo framebuffer_create_info{}; {
            framebuffer_create_info.renderPass = offscreen_render_pass;
            framebuffer_create_info.height = ctx.offscreen.image.height;
            framebuffer_create_info.width = ctx.offscreen.image.width;
            framebuffer_create_info.layers = 1;
            framebuffer_create_info.attachmentCount = attachments.size();
            framebuffer_create_info.pAttachments = attachments.data();
        }

        framebuffer = ctx.device.logical.createFramebuffer(framebuffer_create_info, nullptr, ctx.dispatcher);

        logger::info("Framebuffer successfully created with offscreen renderpass");

        return framebuffer;
    }
} // namespace tethys::api