#ifndef TETHYS_RENDERER_HPP
#define TETHYS_RENDERER_HPP

#include <tethys/renderer/DrawCommand.hpp>
#include <tethys/Forwards.hpp>
#include <tethys/Handle.hpp>
#include <tethys/Mesh.hpp>

#include <vector>

namespace tethys::renderer {
    void initialise();

    [[nodiscard]] Handle<Mesh> upload(Mesh&&);
    [[nodiscard]] std::vector<Handle<Mesh>> upload(std::vector<Mesh>&&);

    void start();
    void draw(const std::vector<DrawCommand>&);
    void end();
    void submit();
} // namespace tethys::renderer

#endif //TETHYS_RENDERER_HPP
