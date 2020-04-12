#include <tethys/api/private/static_buffer.hpp>
#include <tethys/api/private/context.hpp>
#include <tethys/Texture.hpp>
#include <tethys/Logger.hpp>

#include <Texas/Texas.hpp>

#include <vulkan/vulkan.hpp>

#include <fstream>
#include <string>

namespace tethys {
    class StupidAllocator : public Texas::Allocator { // Really?
    public:
        std::byte* allocate(std::size_t amount, MemoryType) override {
            return new std::byte[amount];
        }

        void deallocate(std::byte* ptr, MemoryType) override {
            delete[] ptr;
        }
    };

    void Texture::load(const char* path) {
        using namespace std::string_literals;
        using namespace api;

        if (!std::ifstream(path).is_open()) {
            throw std::runtime_error("File not found error at: "s + path);
        }

        StupidAllocator allocator{};

        auto result = Texas::loadFromPath(path, allocator);

        logger::info("Loading texture: "s + path);

        if (!result) {
            throw std::runtime_error("Failed to load texture!");
        }

        auto texture = std::move(result.value());

        auto dimensions = texture.baseDimensions();
        auto texture_size = dimensions.height * dimensions.width * dimensions.depth;

        auto staging = api::make_buffer(texture_size, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT);

        void* mapped{};
        vmaMapMemory(ctx.allocator, staging.allocation, &mapped);
        std::memcpy(mapped, texture.rawBufferSpan().data(), texture_size);
        vmaUnmapMemory(ctx.allocator, staging.allocation);

        api::Image::CreateInfo create_info{}; {
            create_info.width = dimensions.width;
            create_info.height = dimensions.height;
            create_info.memory_usage = VMA_MEMORY_USAGE_GPU_ONLY;
            create_info.usage_flags = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
            create_info.format = vk::Format::eR8G8B8A8Unorm;
            create_info.tiling = vk::ImageTiling::eOptimal;
        }

        image = api::make_image(create_info);

        api::transition_image_layout(image.handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        api::copy_buffer_to_image(staging.handle, image.handle, dimensions.width, dimensions.height);
        api::transition_image_layout(image.handle, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

        view = api::make_image_view(image.handle, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);

        logger::info("Successfully loaded texture, "
                     "width: ", dimensions.width,
                     ", height: ", dimensions.height,
                     ", channels: ", dimensions.depth);
    }
} // namespace tethys