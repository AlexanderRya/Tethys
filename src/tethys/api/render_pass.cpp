#include <tethys/api/render_target.hpp>
#include <tethys/api/render_pass.hpp>
#include <tethys/api/context.hpp>
#include <tethys/logger.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    vk::RenderPass make_offscreen_render_pass(const Offscreen& offscreen) {
        std::array<vk::AttachmentDescription, 3> attachments{}; {
            attachments[0].format = offscreen.msaa.format;
            attachments[0].samples = context.device.samples;
            attachments[0].loadOp = vk::AttachmentLoadOp::eClear;
            attachments[0].storeOp = vk::AttachmentStoreOp::eStore;
            attachments[0].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            attachments[0].initialLayout = vk::ImageLayout::eUndefined;
            attachments[0].finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

            attachments[1].format = offscreen.depth.format;
            attachments[1].samples = context.device.samples;
            attachments[1].loadOp = vk::AttachmentLoadOp::eClear;
            attachments[1].storeOp = vk::AttachmentStoreOp::eDontCare;
            attachments[1].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            attachments[1].initialLayout = vk::ImageLayout::eUndefined;
            attachments[1].finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

            attachments[2].format = offscreen.color.format;
            attachments[2].samples = vk::SampleCountFlagBits::e1;
            attachments[2].loadOp = vk::AttachmentLoadOp::eDontCare;
            attachments[2].storeOp = vk::AttachmentStoreOp::eStore;
            attachments[2].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            attachments[2].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            attachments[2].initialLayout = vk::ImageLayout::eUndefined;
            attachments[2].finalLayout = vk::ImageLayout::eTransferSrcOptimal;
        }

        vk::AttachmentReference color_attachment{}; {
            color_attachment.layout = vk::ImageLayout::eColorAttachmentOptimal;
            color_attachment.attachment = 0;
        }

        vk::AttachmentReference depth_attachment{}; {
            depth_attachment.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            depth_attachment.attachment = 1;
        }

        vk::AttachmentReference color_resolve_attachment{}; {
            color_resolve_attachment.layout = vk::ImageLayout::eColorAttachmentOptimal;
            color_resolve_attachment.attachment = 2;
        }

        vk::SubpassDescription subpass_description{}; {
            subpass_description.colorAttachmentCount = 1;
            subpass_description.pColorAttachments = &color_attachment;
            subpass_description.pDepthStencilAttachment = &depth_attachment;
            subpass_description.pResolveAttachments = &color_resolve_attachment;
            subpass_description.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        }

        vk::SubpassDependency subpass_dependency{}; {
            subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            subpass_dependency.dstSubpass = 0;
            subpass_dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            subpass_dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            subpass_dependency.srcAccessMask = {};
            subpass_dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
        }

        vk::RenderPassCreateInfo render_pass_create_info{}; {
            render_pass_create_info.attachmentCount = attachments.size();
            render_pass_create_info.pAttachments = attachments.data();
            render_pass_create_info.subpassCount = 1;
            render_pass_create_info.pSubpasses = &subpass_description;
            render_pass_create_info.dependencyCount = 1;
            render_pass_create_info.pDependencies = &subpass_dependency;
        }

        auto render_pass = context.device.logical.createRenderPass(render_pass_create_info, nullptr, context.dispatcher);

        logger::info("Offscreen renderpass successfully created");

        return render_pass;
    }
} // namespace tethys::api