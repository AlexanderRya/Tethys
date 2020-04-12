#ifndef TETHYS_RENDER_DATA_HPP
#define TETHYS_RENDER_DATA_HPP

#include <tethys/forwards.hpp>
#include <tethys/handle.hpp>
#include <tethys/mesh.hpp>

#include <glm/mat4x4.hpp>

namespace tethys::renderer {
    struct RenderData {
        struct DrawCommand {
            Handle<Mesh> mesh;
            Handle<Texture> texture;
            glm::mat4 transform;
        };

        std::vector<DrawCommand> commands;

        glm::mat4 pv_matrix;
    };
} // namespace tethys::renderer

#endif //TETHYS_RENDER_DATA_HPP
