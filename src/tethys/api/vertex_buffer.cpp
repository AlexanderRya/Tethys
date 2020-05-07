#include <tethys/api/static_buffer.hpp>
#include <tethys/api/vertex_buffer.hpp>
#include <tethys/api/context.hpp>
#include <tethys/vertex.hpp>
#include <tethys/logger.hpp>

namespace tethys::api {
    VertexBuffer make_vertex_buffer(const std::vector<Vertex>& vertices) {
        StaticBuffer temp_buffer;
        // Allocate staging buffer
        temp_buffer = make_buffer(
            vertices.size() * sizeof(Vertex),
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_MEMORY_USAGE_CPU_ONLY,
            VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT);

        void* mapped{};
        vmaMapMemory(context.allocator, temp_buffer.allocation, &mapped);
        std::memcpy(mapped, vertices.data(), sizeof(Vertex) * vertices.size());
        vmaUnmapMemory(context.allocator, temp_buffer.allocation);

        VertexBuffer vertex_buffer;
        // Allocate device local buffer
        vertex_buffer.buffer = make_buffer(
            vertices.size() * sizeof(Vertex),
            vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
            VMA_MEMORY_USAGE_GPU_ONLY,
            VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT);

        // Copy to device local
        api::copy_buffer(temp_buffer.handle, vertex_buffer.buffer.handle, vertices.size() * sizeof(Vertex));

        vmaDestroyBuffer(context.allocator, temp_buffer.handle, temp_buffer.allocation);

        logger::info("Allocated vertex buffer with size (in bytes): {}", vertices.size() * sizeof(Vertex));

        vertex_buffer.size = vertices.size();

        return vertex_buffer;
    }
} // namespace tethys::api