#ifndef TETHYS_DIRECTIONAL_LIGHT_HPP
#define TETHYS_DIRECTIONAL_LIGHT_HPP

#include <glm/vec3.hpp>

namespace tethys {
    struct DirectionalLight {
        alignas(16) glm::vec3 direction;
        alignas(16) glm::vec3 color;
    };

    static_assert(sizeof(DirectionalLight) == static_cast<usize>(32), "Nope, doesn't work!");
} // namespace tethys

#endif //TETHYS_DIRECTIONAL_LIGHT_HPP
