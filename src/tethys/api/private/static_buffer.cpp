#include <tethys/api/private/CommandBuffer.hpp>
#include <tethys/api/private/StaticBuffer.hpp>
#include <tethys/api/private/Context.hpp>
#include <tethys/api/private/Device.hpp>

namespace tethys::api {
    StaticBuffer make_buffer(const usize size, const vk::BufferUsageFlags& usage, const VmaMemoryUsage memory_usage, const VmaAllocationCreateFlags alloc_flags) {
        vk::BufferCreateInfo buffer_create_info{}; {
            buffer_create_info.size = size;
            buffer_create_info.queueFamilyIndexCount = 1;
            buffer_create_info.pQueueFamilyIndices = &ctx.device.queue_family;
            buffer_create_info.usage = usage;
            buffer_create_info.sharingMode = vk::SharingMode::eExclusive;
        }

        VmaAllocationCreateInfo allocation_create_info{}; {
            allocation_create_info.flags = alloc_flags;
            allocation_create_info.requiredFlags = 0;
            allocation_create_info.preferredFlags = 0;
            allocation_create_info.memoryTypeBits = 0;
            allocation_create_info.pool = nullptr;
            allocation_create_info.pUserData = nullptr;
            allocation_create_info.usage = memory_usage;
        }

        StaticBuffer buffer{};

        vmaCreateBuffer(
            ctx.allocator,
            reinterpret_cast<VkBufferCreateInfo*>(&buffer_create_info),
            &allocation_create_info,
            reinterpret_cast<VkBuffer*>(&buffer.handle),
            &buffer.allocation,
            nullptr);

        buffer.flags = usage;

        return buffer;
    }

    void copy_buffer(const vk::Buffer src, vk::Buffer dst, const usize size) {
        auto command_buffer = begin_transient(); {
            vk::BufferCopy region{}; {
                region.size = size;
                region.srcOffset = 0;
                region.dstOffset = 0;
            }

            command_buffer.copyBuffer(src, dst, region, ctx.dispatcher);

            end_transient(command_buffer);
        }
    }

    void copy_buffer_to_image(const vk::Buffer buffer, vk::Image image, const u32 width, const u32 height) {
        auto command_buffer = begin_transient(); {
            vk::BufferImageCopy region{}; {
                region.bufferOffset = 0;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;

                region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = 0;
                region.imageSubresource.layerCount = 1;

                region.imageOffset = { { 0, 0, 0 } };
                region.imageExtent = { {
                    width,
                    height,
                    1
                } };
            }

            command_buffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region, ctx.dispatcher);

            end_transient(command_buffer);
        }
    }

    void destroy_buffer(StaticBuffer& buffer) {
        vmaDestroyBuffer(ctx.allocator, buffer.handle, buffer.allocation);
    }
} // namespace tethys::api