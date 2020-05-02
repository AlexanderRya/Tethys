#ifndef TETHYS_MESH_HPP
#define TETHYS_MESH_HPP

#include <tethys/vertex.hpp>
#include <tethys/handle.hpp>
#include <tethys/types.hpp>

#include <vector>

namespace tethys {
    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
    };

    [[nodiscard]] Mesh generate_debug_quad();
} // namespace tethys

#endif //TETHYS_MESH_HPP
