#ifndef TETHYS_VERTEXBUFFER_HPP
#define TETHYS_VERTEXBUFFER_HPP

#include <tethys/api/private/StaticBuffer.hpp>
#include <tethys/Forwards.hpp>

#include <vector>

namespace tethys::api {
    struct VertexBuffer {
        StaticBuffer buffer{};
        usize size{};
    };
    [[nodiscard]] VertexBuffer make_vertex_buffer(std::vector<Vertex>&&);
} // namespace tethys::api

#endif //TETHYS_VERTEXBUFFER_HPP
