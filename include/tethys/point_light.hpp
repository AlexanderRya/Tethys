#ifndef TETHYS_POINT_LIGHT_HPP
#define TETHYS_POINT_LIGHT_HPP

#include <glm/vec3.hpp>

namespace tethys {
    struct PointLight {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 color;

        float constant;
        float linear;
        float quadratic;
    };
} // namespace tethys

#endif //TETHYS_POINT_LIGHT_HPP
