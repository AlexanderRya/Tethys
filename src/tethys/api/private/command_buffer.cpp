#include <tethys/api/private/command_buffer.hpp>
#include <tethys/api/private/context.hpp>
#include <tethys/logger.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    std::vector<vk::CommandBuffer> make_rendering_command_buffers() {
        vk::CommandBufferAllocateInfo allocate_info{}; {
            allocate_info.commandPool = ctx.command_pool;
            allocate_info.commandBufferCount = ctx.swapchain.image_count;
            allocate_info.level = vk::CommandBufferLevel::ePrimary;
        }

        auto buffers = ctx.device.logical.allocateCommandBuffers(allocate_info, ctx.dispatcher);

        logger::info("Created ", allocate_info.commandBufferCount, " command buffers for rendering");

        return buffers;
    }

    vk::CommandBuffer begin_transient() {
        vk::CommandBufferAllocateInfo command_buffer_allocate_info{}; {
            command_buffer_allocate_info.commandBufferCount = 1;
            command_buffer_allocate_info.level = vk::CommandBufferLevel::ePrimary;
            command_buffer_allocate_info.commandPool = ctx.transient_pool;
        }

        auto command_buffers = ctx.device.logical.allocateCommandBuffers(command_buffer_allocate_info, ctx.dispatcher);

        vk::CommandBufferBeginInfo begin_info{}; {
            begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        }

        command_buffers[0].begin(begin_info, ctx.dispatcher);

        return command_buffers[0];
    }

    void end_transient(const vk::CommandBuffer command_buffer) {
        command_buffer.end(ctx.dispatcher);

        vk::SubmitInfo submit_info{}; {
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer;
        }

        ctx.device.queue.submit(submit_info, nullptr, ctx.dispatcher);

        ctx.device.queue.waitIdle(ctx.dispatcher);

        ctx.device.logical.freeCommandBuffers(ctx.transient_pool, command_buffer, ctx.dispatcher);
    }
} // namespace tethys::api