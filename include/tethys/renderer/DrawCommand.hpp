#ifndef TETHYS_DRAWCOMMAND_HPP
#define TETHYS_DRAWCOMMAND_HPP

#include <tethys/Handle.hpp>
#include <tethys/Mesh.hpp>

namespace tethys::renderer {
    struct DrawCommand {
        Handle<Mesh> mesh;
    };
} // namespace tethys::renderer

#endif //TETHYS_DRAWCOMMAND_HPP
