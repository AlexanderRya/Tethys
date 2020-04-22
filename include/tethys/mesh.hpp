#ifndef TETHYS_MESH_HPP
#define TETHYS_MESH_HPP

#include <tethys/vertex.hpp>

#include <vector>

namespace tethys {
    struct Mesh {
        std::vector<Vertex> geometry;
        std::vector<u32> indices;
    };
} // namespace tethys

#endif //TETHYS_MESH_HPP
