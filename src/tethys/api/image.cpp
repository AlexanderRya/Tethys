#include <tethys/api/command_buffer.hpp>
#include <tethys/api/context.hpp>
#include <tethys/api/image.hpp>

namespace tethys::api {
    Image make_image(const Image::CreateInfo& info) {
        if (info.samples > context.device.samples) {
            throw std::invalid_argument("Number of samples requested not supported by the GPU");
        }

        vk::ImageCreateInfo image_info{}; {
            image_info.imageType = vk::ImageType::e2D;
            image_info.extent = { {
                static_cast<u32>(info.width),
                static_cast<u32>(info.height),
                1
            } };
            image_info.mipLevels = info.mips;
            image_info.arrayLayers = 1;
            image_info.format = info.format;
            image_info.tiling = info.tiling;
            image_info.initialLayout = vk::ImageLayout::eUndefined;
            image_info.usage = info.usage_flags;
            image_info.samples = info.samples;
            image_info.sharingMode = vk::SharingMode::eExclusive;
        }

        VmaAllocationCreateInfo allocation_create_info{}; {
            allocation_create_info.flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
            allocation_create_info.requiredFlags = 0;
            allocation_create_info.preferredFlags = 0;
            allocation_create_info.memoryTypeBits = 0;
            allocation_create_info.pool = nullptr;
            allocation_create_info.pUserData = nullptr;
            allocation_create_info.usage = info.memory_usage;
        }

        Image image{};

        vmaCreateImage(
            context.allocator,
            reinterpret_cast<VkImageCreateInfo*>(&image_info),
            &allocation_create_info,
            reinterpret_cast<VkImage*>(&image.handle),
            &image.allocation,
            nullptr);

        image.format = info.format;
        image.height = info.height;
        image.width = info.width;
        image.tiling = info.tiling;
        image.samples = info.samples;

        return image;
    }

    vk::ImageView make_image_view(const vk::Image image, const vk::Format format, const vk::ImageAspectFlags aspect, const u32 mips) {
        vk::ImageViewCreateInfo image_view_create_info{}; {
            image_view_create_info.image = image;
            image_view_create_info.format = format;
            image_view_create_info.components.r = vk::ComponentSwizzle::eIdentity;
            image_view_create_info.components.g = vk::ComponentSwizzle::eIdentity;
            image_view_create_info.components.b = vk::ComponentSwizzle::eIdentity;
            image_view_create_info.components.a = vk::ComponentSwizzle::eIdentity;
            image_view_create_info.viewType = vk::ImageViewType::e2D;
            image_view_create_info.subresourceRange.aspectMask = aspect;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = mips;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
        }

        return context.device.logical.createImageView(image_view_create_info, nullptr, context.dispatcher);
    }

    void transition_image_layout(vk::Image image, const vk::ImageLayout old_layout, const vk::ImageLayout new_layout, const u32 mips) {
        auto command_buffer = begin_transient(); {
            vk::ImageMemoryBarrier image_memory_barrier{}; {
                image_memory_barrier.image = image;

                image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                image_memory_barrier.oldLayout = old_layout;
                image_memory_barrier.newLayout = new_layout;

                image_memory_barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                image_memory_barrier.subresourceRange.baseMipLevel = 0;
                image_memory_barrier.subresourceRange.levelCount = mips;
                image_memory_barrier.subresourceRange.baseArrayLayer = 0;
                image_memory_barrier.subresourceRange.layerCount = 1;

                image_memory_barrier.srcAccessMask = {};
                image_memory_barrier.dstAccessMask = {};
            }

            vk::PipelineStageFlags source_stage{};
            vk::PipelineStageFlags destination_stage{};

            switch (old_layout) {
                case vk::ImageLayout::eUndefined: {
                    image_memory_barrier.srcAccessMask = {};
                    source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
                } break;

                case vk::ImageLayout::eTransferDstOptimal: {
                    image_memory_barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                    source_stage = vk::PipelineStageFlagBits::eTransfer;
                } break;

                default: {
                    throw std::runtime_error("Unsupported transition");
                }
            }

            switch (new_layout) {
                case vk::ImageLayout::eTransferDstOptimal: {
                    image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
                    destination_stage = vk::PipelineStageFlagBits::eTransfer;
                } break;

                case vk::ImageLayout::eShaderReadOnlyOptimal: {
                    image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
                    destination_stage = vk::PipelineStageFlagBits::eFragmentShader;
                } break;

                case vk::ImageLayout::eDepthStencilAttachmentOptimal: {
                    image_memory_barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
                    image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                    destination_stage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
                } break;

                case vk::ImageLayout::ePresentSrcKHR: {
                    image_memory_barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
                    destination_stage = vk::PipelineStageFlagBits::eAllGraphics;
                } break;

                default: {
                    throw std::runtime_error("Unsupported transition");
                }
            }

            command_buffer.pipelineBarrier(
                source_stage,
                destination_stage,
                vk::DependencyFlags{},
                nullptr,
                nullptr,
                image_memory_barrier,
                context.dispatcher);

            end_transient(command_buffer);
        }
    }
} // namespace tethys::api