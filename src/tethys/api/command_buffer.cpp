#include <tethys/api/command_buffer.hpp>
#include <tethys/api/context.hpp>
#include <tethys/logger.hpp>
#include <tethys/types.hpp>

#include <vulkan/vulkan.hpp>

namespace tethys::api {
    std::vector<vk::CommandBuffer> make_rendering_command_buffers() {
        vk::CommandBufferAllocateInfo allocate_info{}; {
            allocate_info.commandPool = context.command_pool;
            allocate_info.commandBufferCount = context.swapchain.image_count;
            allocate_info.level = vk::CommandBufferLevel::ePrimary;
        }

        auto buffers = context.device.logical.allocateCommandBuffers(allocate_info, context.dispatcher);

        logger::info("Created ", allocate_info.commandBufferCount, " command buffers for rendering");

        return buffers;
    }

    vk::CommandBuffer begin_transient() {
        vk::CommandBufferAllocateInfo command_buffer_allocate_info{}; {
            command_buffer_allocate_info.commandBufferCount = 1;
            command_buffer_allocate_info.level = vk::CommandBufferLevel::ePrimary;
            command_buffer_allocate_info.commandPool = context.transient_pool;
        }

        auto command_buffers = context.device.logical.allocateCommandBuffers(command_buffer_allocate_info, context.dispatcher);

        vk::CommandBufferBeginInfo begin_info{}; {
            begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        }

        command_buffers[0].begin(begin_info, context.dispatcher);

        return command_buffers[0];
    }

    void end_transient(const vk::CommandBuffer command_buffer) {
        command_buffer.end(context.dispatcher);

        vk::SubmitInfo submit_info{}; {
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer;
        }

        context.device.queue.submit(submit_info, nullptr, context.dispatcher);

        context.device.queue.waitIdle(context.dispatcher);

        context.device.logical.freeCommandBuffers(context.transient_pool, command_buffer, context.dispatcher);
    }
} // namespace tethys::api