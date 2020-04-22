#ifndef TETHYS_RENDERER_HPP
#define TETHYS_RENDERER_HPP

#include <tethys/forwards.hpp>
#include <tethys/vertex.hpp>
#include <tethys/handle.hpp>
#include <tethys/types.hpp>
#include <tethys/mesh.hpp>

#include <vector>

namespace tethys::renderer {
    void initialise();

    [[nodiscard]] Handle<Mesh> write_geometry(std::vector<Vertex>&&, std::vector<u32>&&);
    [[nodiscard]] Handle<Texture> upload_texture(const char*);

    void unload_geometry(Handle<Mesh>&&);

    void start();
    void draw(const RenderData&);
    void end();
    void submit();
} // namespace tethys::renderer

#endif //TETHYS_RENDERER_HPP
