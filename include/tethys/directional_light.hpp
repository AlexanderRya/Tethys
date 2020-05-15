#ifndef TETHYS_DIRECTIONAL_LIGHT_HPP
#define TETHYS_DIRECTIONAL_LIGHT_HPP

#include <glm/vec3.hpp>

namespace tethys {
    struct DirectionalLight {
        glm::vec3 direction;
        float pad0;
        glm::vec3 color;
        float pad1;
    };
} // namespace tethys

#endif //TETHYS_DIRECTIONAL_LIGHT_HPP
