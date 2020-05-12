#ifndef TETHYS_RENDER_DATA_HPP
#define TETHYS_RENDER_DATA_HPP

#include <tethys/pipeline.hpp>
#include <tethys/forwards.hpp>
#include <tethys/camera.hpp>
#include <tethys/model.hpp>
#include <tethys/mesh.hpp>

#include <glm/mat4x4.hpp>

namespace tethys {
    struct DrawCommand {
        Model model{};
        glm::mat4 transform{};
        Pipeline shader{};
    };

    struct RenderData {
        std::vector<DrawCommand> draw_commands{};

        std::vector<PointLight> point_lights{};
        std::vector<DirectionalLight> directional_lights{};

        Camera camera{}; // Just one for now
    };
} // namespace tethys

#endif //TETHYS_RENDER_DATA_HPP
