#ifndef TETHYS_RENDER_DATA_HPP
#define TETHYS_RENDER_DATA_HPP

#include <tethys/forwards.hpp>
#include <tethys/camera.hpp>
#include <tethys/handle.hpp>
#include <tethys/mesh.hpp>

#include <glm/mat4x4.hpp>

namespace tethys {
    struct RenderData {
        struct DrawCommand {
            Handle<Mesh> mesh;
            Handle<Texture> texture;
            glm::mat4 transform;
        };

        std::vector<DrawCommand> commands;

        std::vector<PointLight> point_lights;

        Camera camera; // Just one for now
    };
} // namespace tethys

#endif //TETHYS_RENDER_DATA_HPP
