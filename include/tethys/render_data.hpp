#ifndef TETHYS_RENDER_DATA_HPP
#define TETHYS_RENDER_DATA_HPP

#include <tethys/forwards.hpp>
#include <tethys/material.hpp>
#include <tethys/camera.hpp>
#include <tethys/handle.hpp>
#include <tethys/mesh.hpp>

#include <glm/mat4x4.hpp>

namespace tethys {
    struct DrawCommand{
        Handle<Model> model{};
        glm::mat4 transform{};
        u32 shader{};
    };

    struct RenderData {
        std::vector<DrawCommand> draw_commands{};

        std::vector<PointLight> point_lights{};
        std::vector<DirectionalLight> directional_lights{};

        Camera camera{}; // Just one for now
    };
} // namespace tethys

#endif //TETHYS_RENDER_DATA_HPP
