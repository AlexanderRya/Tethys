#include <tethys/api/command_buffer.hpp>
#include <tethys/api/static_buffer.hpp>
#include <tethys/api/context.hpp>
#include <tethys/api/sampler.hpp>
#include <tethys/texture.hpp>

#include <tethys/logger.hpp>

#include <stb_image.h>

#include <vulkan/vulkan.hpp>
#include <fstream>
#include <string>
#include <cmath>

namespace tethys {
    using namespace api;

    static void generate_mipmaps(const Texture& texture) {
        auto command_buffer = api::begin_transient();

        vk::ImageMemoryBarrier barrier{}; {
            barrier.image = texture.image.handle;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;
        }

        i32 mip_width = texture.image.width;
        i32 mip_height = texture.image.height;

        for (usize i = 1; i < texture.mips; i++) {
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.subresourceRange.baseMipLevel = i - 1;

            command_buffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlagBits{},
                nullptr,
                nullptr,
                barrier,
                ctx.dispatcher);

            vk::ImageBlit blit{}; {
                blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
                blit.srcOffsets[1] = vk::Offset3D{ mip_width, mip_height, 1 };
                blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
                blit.dstOffsets[1] = vk::Offset3D{ mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
                blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;
            }

            command_buffer.blitImage(
                texture.image.handle, vk::ImageLayout::eTransferSrcOptimal,
                texture.image.handle, vk::ImageLayout::eTransferDstOptimal,
                blit,vk::Filter::eLinear,
                ctx.dispatcher);

            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            command_buffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eFragmentShader,
                vk::DependencyFlagBits{},
                nullptr,
                nullptr,
                barrier,
                ctx.dispatcher);

            if (mip_width > 1) {
                mip_width /= 2;
            }
            if (mip_height > 1) {
                mip_height /= 2;
            }
        }

        barrier.subresourceRange.baseMipLevel = texture.mips - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            vk::DependencyFlagBits{},
            nullptr,
            nullptr,
            barrier,
            ctx.dispatcher);

        end_transient(command_buffer);
    }

    Texture load_texture(const char* path, const vk::Format format) {
        using namespace std::string_literals;

        if (!std::ifstream(path).is_open()) {
            throw std::runtime_error("File not found error at: "s + path);
        }

        i32 width, height, channels = 4;

        logger::info("Loading texture: "s + path);

        auto data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
        auto texture = load_texture(data, width, height, 4, format);
        stbi_image_free(data);

        return texture;
    }

    Texture load_texture(const u8* data, const u32 width, const u32 height, const u32 channels, const vk::Format format) {
        if (!data) {
            throw std::runtime_error("Error, can't load texture without data");
        }

        if (width <= 0 || height <= 0 || channels <= 0) {
            throw std::runtime_error("wtf are you doing");
        }

        auto texture_size = width * height * channels;

        auto staging = api::make_buffer(texture_size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT);

        void* mapped{};
        vmaMapMemory(ctx.allocator, staging.allocation, &mapped);
        std::memcpy(mapped, data, texture_size);
        vmaUnmapMemory(ctx.allocator, staging.allocation);

        Texture texture;
        texture.mips = std::floor(std::log2(std::max(width, height)));

        if (texture.mips == 0) {
            texture.mips = 1;
        }

        api::Image::CreateInfo create_info{}; {
            create_info.width = width;
            create_info.height = height;
            create_info.mips = texture.mips;
            create_info.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
            create_info.usage_flags = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
            create_info.format = format;
            create_info.tiling = vk::ImageTiling::eOptimal;
            create_info.samples = vk::SampleCountFlagBits::e1;
        }

        texture.image = api::make_image(create_info);

        api::transition_image_layout(texture.image.handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, texture.mips);
        api::copy_buffer_to_image(staging.handle, texture.image.handle, width, height);
        generate_mipmaps(texture);

        texture.view = api::make_image_view(texture.image.handle, format, vk::ImageAspectFlagBits::eColor, texture.mips);

        logger::info("Successfully loaded texture, "
                     "width: ", width,
                     ", height: ", height,
                     ", channels: ", channels);

        return texture;
    }

    vk::DescriptorImageInfo Texture::info(const api::SamplerType& type) const {
        vk::DescriptorImageInfo image_info{}; {
            image_info.sampler = api::sampler_from_type(type);
            image_info.imageView = view;
            image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }
        return image_info;
    }
} // namespace tethys