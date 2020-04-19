#ifndef TETHYS_POINT_LIGHT_HPP
#define TETHYS_POINT_LIGHT_HPP

#include <glm/vec3.hpp>

namespace tethys {
    struct PointLight { // Aligned as 12?
        alignas(16) glm::vec3 position; // Offset 12 (? I guess?)
        alignas(16) glm::vec3 ambient;
        alignas(16) glm::vec3 diffuse;
        alignas(16) glm::vec3 specular;

        float constant;
        float linear;
        float quadratic;
    };
} // namespace tethys

#endif //TETHYS_POINT_LIGHT_HPP
