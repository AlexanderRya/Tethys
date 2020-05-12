#ifndef TETHYS_MESH_HPP
#define TETHYS_MESH_HPP

#include <tethys/api/vertex_buffer.hpp>
#include <tethys/api/index_buffer.hpp>
#include <tethys/vertex.hpp>
#include <tethys/handle.hpp>
#include <tethys/types.hpp>

#include <vector>

namespace tethys {
    struct VertexData {
        std::vector<Vertex> geometry{};
        std::vector<u32> indices{};
    };

    struct Mesh {
        api::VertexBuffer vbo;
        api::IndexBuffer ibo;

        usize vertex_count;
        usize index_count;
    };

    [[nodiscard]] VertexData generate_sphere(const f32, const u32, const u32);
} // namespace tethys

#endif //TETHYS_MESH_HPP
