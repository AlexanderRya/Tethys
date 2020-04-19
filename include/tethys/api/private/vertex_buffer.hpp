#ifndef TETHYS_VERTEX_BUFFER_HPP
#define TETHYS_VERTEX_BUFFER_HPP

#include <tethys/api/private/static_buffer.hpp>
#include <tethys/forwards.hpp>

#include <vector>

namespace tethys::api {
    struct VertexBuffer {
        StaticBuffer buffer{};
        usize size{};
    };
    [[nodiscard]] VertexBuffer make_vertex_buffer(std::vector<Vertex>&&);
} // namespace tethys::api

#endif //TETHYS_VERTEX_BUFFER_HPP
