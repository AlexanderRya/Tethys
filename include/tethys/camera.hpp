#ifndef TETHYS_CAMERA_HPP
#define TETHYS_CAMERA_HPP

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace tethys {
    struct Camera {
        glm::mat4 pv_mat;
        glm::vec4 pos;
    };
} // namespace tethys

#endif //TETHYS_CAMERA_HPP
