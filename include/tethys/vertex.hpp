#ifndef TETHYS_VERTEX_HPP
#define TETHYS_VERTEX_HPP

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace tethys {
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 norms;
        glm::vec2 uvs;
        glm::vec3 tangent;
        glm::vec3 bitangent;
    };
} // namespace tethys

#endif //TETHYS_VERTEX_HPP
