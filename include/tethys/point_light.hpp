#ifndef TETHYS_POINT_LIGHT_HPP
#define TETHYS_POINT_LIGHT_HPP

#include <glm/vec3.hpp>

namespace tethys {
    struct Falloff {
        float intensity;
        float constant;
        float linear;
        float quadratic;
    };

    static_assert(sizeof(Falloff) == static_cast<usize>(16), "Nope, doesn't work!");

    struct PointLight {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 color;
        alignas(16) Falloff falloff;
    };

    static_assert(sizeof(PointLight) == static_cast<usize>(48), "Nope, doesn't work!");
} // namespace tethys

#endif //TETHYS_POINT_LIGHT_HPP
