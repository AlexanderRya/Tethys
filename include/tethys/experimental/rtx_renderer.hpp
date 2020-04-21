#ifndef TETHYS_RTX_RENDERER_HPP
#define TETHYS_RTX_RENDERER_HPP

#include <tethys/forwards.hpp>

#include <vector>

namespace tethys::experimental::renderer {
    void initialise();
    Handle<Mesh> upload(std::vector<Vertex>&&);
} // namespace tethys::experimental::renderer

#endif //TETHYS_RTX_RENDERER_HPP
