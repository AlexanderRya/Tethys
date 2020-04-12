#include <tethys/api/private/static_buffer.hpp>
#include <tethys/api/private/context.hpp>
#include <tethys/texture.hpp>
#include <tethys/logger.hpp>

#include <stb_image.h>

#include <vulkan/vulkan.hpp>

#include <fstream>
#include <string>

namespace tethys {
    void Texture::load(const char* path) {
        using namespace std::string_literals;
        using namespace api;

        if (!std::ifstream(path).is_open()) {
            throw std::runtime_error("File not found error at: "s + path);
        }

        i32 width, height, channels = 4;

        auto data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

        usize texture_size = width * height * 4;

        logger::info("Loading texture: "s + path);

        if (!data) {
            throw std::runtime_error("Failed to load texture!");
        }

        auto staging = api::make_buffer(texture_size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT);

        void* mapped{};
        vmaMapMemory(ctx.allocator, staging.allocation, &mapped);
        std::memcpy(mapped, data, texture_size);
        vmaUnmapMemory(ctx.allocator, staging.allocation);

        api::Image::CreateInfo create_info{}; {
            create_info.width = width;
            create_info.height = height;
            create_info.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
            create_info.usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
            create_info.format = vk::Format::eR8G8B8A8Unorm;
            create_info.tiling = vk::ImageTiling::eOptimal;
        }

        image = api::make_image(create_info);

        api::transition_image_layout(image.handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        api::copy_buffer_to_image(staging.handle, image.handle, width, height);
        api::transition_image_layout(image.handle, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        view = api::make_image_view(image.handle, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);

        logger::info("Successfully loaded texture, "
                     "width: ", width,
                     ", height: ", height,
                     ", channels: ", channels);
    }

    vk::DescriptorImageInfo Texture::info() const {
        using namespace api;

        vk::DescriptorImageInfo image_info{}; {
            image_info.sampler = ctx.default_sampler;
            image_info.imageView = view;
            image_info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        }
        return image_info;
    }
} // namespace tethys