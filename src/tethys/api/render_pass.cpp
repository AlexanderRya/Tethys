#include <tethys/api/render_pass.hpp>
#include <tethys/api/context.hpp>
#include <tethys/logger.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    vk::RenderPass make_default_render_pass() {
        vk::AttachmentDescription color_description{}; {
            color_description.format = ctx.swapchain.format.format;
            color_description.samples = ctx.device.max_samples;
            color_description.loadOp = vk::AttachmentLoadOp::eClear;
            color_description.storeOp = vk::AttachmentStoreOp::eStore;
            color_description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            color_description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            color_description.initialLayout = vk::ImageLayout::eUndefined;
            color_description.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
        }

        vk::AttachmentReference color_attachment{}; {
            color_attachment.layout = vk::ImageLayout::eColorAttachmentOptimal;
            color_attachment.attachment = 0;
        }

        vk::AttachmentDescription depth_description{}; {
            depth_description.format = vk::Format::eD32SfloatS8Uint;
            depth_description.samples = ctx.device.max_samples;
            depth_description.loadOp = vk::AttachmentLoadOp::eClear;
            depth_description.storeOp = vk::AttachmentStoreOp::eDontCare;
            depth_description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            depth_description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            depth_description.initialLayout = vk::ImageLayout::eUndefined;
            depth_description.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        }

        vk::AttachmentReference depth_attachment{}; {
            depth_attachment.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            depth_attachment.attachment = 1;
        }

        vk::AttachmentDescription color_resolve_description{}; {
            color_resolve_description.format = ctx.swapchain.format.format;
            color_resolve_description.samples = vk::SampleCountFlagBits::e1;
            color_resolve_description.loadOp = vk::AttachmentLoadOp::eDontCare;
            color_resolve_description.storeOp = vk::AttachmentStoreOp::eStore;
            color_resolve_description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
            color_resolve_description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
            color_resolve_description.initialLayout = vk::ImageLayout::eUndefined;
            color_resolve_description.finalLayout = vk::ImageLayout::ePresentSrcKHR;
        }

        vk::AttachmentReference color_resolve_attachment{}; {
            color_resolve_attachment.layout = vk::ImageLayout::eColorAttachmentOptimal;
            color_resolve_attachment.attachment = 2;
        }

        vk::SubpassDescription subpass_description{}; {
            subpass_description.colorAttachmentCount = 1;
            subpass_description.pColorAttachments = &color_attachment;
            subpass_description.pResolveAttachments = &color_resolve_attachment;
            subpass_description.pDepthStencilAttachment = &depth_attachment;
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

        std::array<vk::AttachmentDescription, 3> attachments {
            color_description,
            depth_description,
            color_resolve_description
        };

        vk::RenderPassCreateInfo render_pass_create_info{}; {
            render_pass_create_info.attachmentCount = attachments.size();
            render_pass_create_info.pAttachments = attachments.data();
            render_pass_create_info.subpassCount = 1;
            render_pass_create_info.pSubpasses = &subpass_description;
            render_pass_create_info.dependencyCount = 1;
            render_pass_create_info.pDependencies = &subpass_dependency;
        }

        auto render_pass = ctx.device.logical.createRenderPass(render_pass_create_info, nullptr, ctx.dispatcher);

        logger::info("Default renderpass successfully created");

        return render_pass;
    }
} // namespace tethys::api