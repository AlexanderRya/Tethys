#ifndef TETHYS_POINT_LIGHT_HPP
#define TETHYS_POINT_LIGHT_HPP

#include <glm/vec3.hpp>

namespace tethys {
    struct PointLight {
        glm::vec3 position;
        float pad0 = 1.0f;
        glm::vec3 color;
        float pad1 = 1.0f;
        float intensity;
        float constant;
        float linear;
        float quadratic;
    };
} // namespace tethys

#endif //TETHYS_POINT_LIGHT_HPP
