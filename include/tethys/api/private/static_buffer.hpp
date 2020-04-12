#ifndef TETHYS_STATICBUFFER_HPP
#define TETHYS_STATICBUFFER_HPP

#include <tethys/Types.hpp>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace tethys::api {
    struct StaticBuffer {
        vk::Buffer handle{};
        vk::BufferUsageFlags flags{};
        VmaAllocation allocation{};
    };

    [[nodiscard]] StaticBuffer make_buffer(const usize, const vk::BufferUsageFlags&, const VmaMemoryUsage, const VmaAllocationCreateFlags);
    void copy_buffer(const vk::Buffer, vk::Buffer, const usize);
    void copy_buffer_to_image(const vk::Buffer, vk::Image, const u32, const u32);
    void destroy_buffer(StaticBuffer& buffer);
} // namespace tethys::api

#endif //TETHYS_STATICBUFFER_HPP
