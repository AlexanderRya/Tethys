#include <tethys/api/render_target.hpp>
#include <tethys/api/framebuffer.hpp>
#include <tethys/api/context.hpp>
#include <tethys/logger.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    vk::Framebuffer make_offscreen_framebuffer(const Offscreen& offscreen, const vk::RenderPass render_pass) {
        std::array<vk::ImageView, 3> attachments{}; {
            attachments[0] = offscreen.resolve_view;
            attachments[1] = offscreen.depth_view;
            attachments[2] = offscreen.color_view;
        }

        vk::FramebufferCreateInfo framebuffer_create_info{}; {
            framebuffer_create_info.renderPass = render_pass;
            framebuffer_create_info.height = offscreen.color.height;
            framebuffer_create_info.width = offscreen.color.width;
            framebuffer_create_info.layers = 1;
            framebuffer_create_info.attachmentCount = attachments.size();
            framebuffer_create_info.pAttachments = attachments.data();
        }

        auto framebuffer = context.device.logical.createFramebuffer(framebuffer_create_info, nullptr, context.dispatcher);

        logger::info("Framebuffer successfully created with offscreen renderpass");

        return framebuffer;
    }
} // namespace tethys::api