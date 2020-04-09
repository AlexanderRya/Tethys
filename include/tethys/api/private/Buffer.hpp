#ifndef TETHYS_BUFFER_HPP
#define TETHYS_BUFFER_HPP

#include <tethys/Types.hpp>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace tethys::api {
    struct Buffer {
        vk::Buffer handle{};
        VmaAllocation allocation{};
        usize size{};
    };

    [[nodiscard]] Buffer make_buffer(const usize, const vk::BufferUsageFlags&, const VmaMemoryUsage, const VmaAllocationCreateFlags);
    [[nodiscard]] vk::Buffer make_buffer(const usize, const vk::BufferUsageFlags);
    [[nodiscard]] vk::DeviceMemory allocate_memory(const vk::Buffer, const vk::MemoryPropertyFlags);
    [[nodiscard]] vk::DeviceMemory allocate_memory(const vk::Image, const vk::MemoryPropertyFlags);
    void copy_buffer(const vk::Buffer, vk::Buffer, const usize);
    void copy_buffer_to_image(const vk::Buffer, vk::Image, const u32, const u32);
} // namespace tethys::api

#endif //TETHYS_BUFFER_HPP
