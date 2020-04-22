#include <tethys/api/static_buffer.hpp>
#include <tethys/api/index_buffer.hpp>
#include <tethys/api/context.hpp>
#include <tethys/logger.hpp>

namespace tethys::api {
    IndexBuffer make_index_buffer(const std::vector<u32>& indices) {
        StaticBuffer temp_buffer;
        // Allocate staging buffer
        temp_buffer = make_buffer(
            indices.size() * sizeof(u32),
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_MEMORY_USAGE_CPU_ONLY,
            VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT);

        void* mapped{};
        vmaMapMemory(ctx.allocator, temp_buffer.allocation, &mapped);
        std::memcpy(mapped, indices.data(), sizeof(u32) * indices.size());
        vmaUnmapMemory(ctx.allocator, temp_buffer.allocation);

        IndexBuffer index_buffer;
        // Allocate device local buffer
        index_buffer.buffer = make_buffer(
            indices.size() * sizeof(u32),
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
            VMA_MEMORY_USAGE_GPU_ONLY,
            VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT);

        // Copy to device local
        api::copy_buffer(temp_buffer.handle, index_buffer.buffer.handle, indices.size() * sizeof(u32));

        vmaDestroyBuffer(ctx.allocator, temp_buffer.handle, temp_buffer.allocation);

        logger::info("Allocated index buffer with size (in bytes): ", indices.size() * sizeof(u32));

        index_buffer.size = indices.size();

        return index_buffer;
    }
} // namespace tethys::api