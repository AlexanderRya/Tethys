#ifndef TETHYS_VERTEXBUFFER_HPP
#define TETHYS_VERTEXBUFFER_HPP

#include <tethys/Forwards.hpp>

#include <vector>

namespace tethys::api {
    [[nodiscard]] Buffer make_vertex_buffer(std::vector<Vertex>&&);
} // namespace tethys::api

#endif //TETHYS_VERTEXBUFFER_HPP
