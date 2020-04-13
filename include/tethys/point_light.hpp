#ifndef TETHYS_POINT_LIGHT_HPP
#define TETHYS_POINT_LIGHT_HPP

#include <glm/vec3.hpp>

namespace tethys {
    struct alignas(16) PointLight {
        glm::vec3 position;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };
} // namespace tethys

#endif //TETHYS_POINT_LIGHT_HPP
