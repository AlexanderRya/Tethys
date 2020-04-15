#ifndef TETHYS_RENDERER_HPP
#define TETHYS_RENDERER_HPP

#include <tethys/render_data.hpp>
#include <tethys/forwards.hpp>
#include <tethys/handle.hpp>
#include <tethys/mesh.hpp>

#include <vector>

namespace tethys::renderer {
    void initialise();

    [[nodiscard]] Handle<Mesh> upload(std::vector<Vertex>&&);
    [[nodiscard]] Handle<Texture> upload(const char*);
    void unload(Handle<Mesh>&&);

    void start();
    void draw(const RenderData&);
    void end();
    void submit();
} // namespace tethys::renderer

#endif //TETHYS_RENDERER_HPP
