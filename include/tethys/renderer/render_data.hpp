#ifndef TETHYS_RENDERDATA_HPP
#define TETHYS_RENDERDATA_HPP

#include <tethys/Handle.hpp>
#include <tethys/Mesh.hpp>

#include <glm/mat4x4.hpp>

namespace tethys::renderer {
    struct RenderData {
        struct DrawCommand {
            Handle<Mesh> mesh;
            glm::mat4 transform;
        };

        std::vector<DrawCommand> commands;

        glm::mat4 pv_matrix;
    };
} // namespace tethys::renderer

#endif //TETHYS_RENDERDATA_HPP
