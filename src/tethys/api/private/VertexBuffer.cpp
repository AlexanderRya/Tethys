#include <tethys/api/private/VertexBuffer.hpp>
#include <tethys/api/private/Context.hpp>
#include <tethys/api/private/Buffer.hpp>
#include <tethys/Vertex.hpp>
#include <tethys/Logger.hpp>

namespace tethys::api {
    Buffer make_vertex_buffer(std::vector<Vertex>&& vertices) {
        Buffer temp_buffer;
        // Allocate staging buffer
        temp_buffer = make_buffer(
            vertices.size() * sizeof(Vertex),
            vk::BufferUsageFlagBits::eTransferSrc,
            VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY,
            VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT);

        void* mapped{};
        vmaMapMemory(ctx.allocator, temp_buffer.allocation, &mapped);
        std::memcpy(mapped, vertices.data(), sizeof(Vertex) * vertices.size());
        vmaUnmapMemory(ctx.allocator, temp_buffer.allocation);

        Buffer vertex_buffer;
        // Allocate device local buffer
        vertex_buffer = make_buffer(
            vertices.size() * sizeof(Vertex),
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
            VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY,
            VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT);

        // Copy to device local
        api::copy_buffer(temp_buffer.handle, vertex_buffer.handle, vertices.size() * sizeof(Vertex));

        vmaDestroyBuffer(ctx.allocator, temp_buffer.handle, temp_buffer.allocation);

        logger::info("Allocated vertex buffer with size (in bytes): ", vertices.size() * sizeof(Vertex));

        vertex_buffer.size = vertices.size();

        return vertex_buffer;
    }
} // namespace tethys::api