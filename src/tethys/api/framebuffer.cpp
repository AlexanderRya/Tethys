#include <tethys/api/render_target.hpp>
#include <tethys/api/framebuffer.hpp>
#include <tethys/api/context.hpp>
#include <tethys/logger.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    vk::Framebuffer make_offscreen_framebuffer(const Offscreen& offscreen, const vk::RenderPass render_pass) {
        std::array<vk::ImageView, 3> attachments{}; {
            attachments[0] = offscreen.msaa_view;
            attachments[1] = offscreen.depth_view;
            attachments[2] = offscreen.image_view;
        }

        vk::FramebufferCreateInfo framebuffer_create_info{}; {
            framebuffer_create_info.renderPass = render_pass;
            framebuffer_create_info.height = offscreen.image.height;
            framebuffer_create_info.width = offscreen.image.width;
            framebuffer_create_info.layers = 1;
            framebuffer_create_info.attachmentCount = attachments.size();
            framebuffer_create_info.pAttachments = attachments.data();
        }

        auto framebuffer = context.device.logical.createFramebuffer(framebuffer_create_info, nullptr, context.dispatcher);

        logger::info("Framebuffer successfully created with offscreen renderpass");

        return framebuffer;
    }

    vk::Framebuffer make_shadow_depth_framebuffer(const ShadowDepth& shadow_depth, const vk::RenderPass render_pass) {
        vk::FramebufferCreateInfo framebuffer_create_info{}; {
            framebuffer_create_info.renderPass = render_pass;
            framebuffer_create_info.width = shadow_depth.image.width;
            framebuffer_create_info.height = shadow_depth.image.height;
            framebuffer_create_info.layers = 1;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments = &shadow_depth.view;
        }

        auto framebuffer = context.device.logical.createFramebuffer(framebuffer_create_info, nullptr, context.dispatcher);

        logger::info("Framebuffer successfully created with shadow depth renderpass");

        return framebuffer;
    }
} // namespace tethys::api