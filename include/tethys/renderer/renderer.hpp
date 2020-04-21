#ifndef TETHYS_RENDERER_HPP
#define TETHYS_RENDERER_HPP

#include <tethys/forwards.hpp>
#include <tethys/types.hpp>

#include <vector>

namespace tethys::renderer {
    void initialise();

    template <typename Ty>
    [[nodiscard]] Handle<Ty> upload(std::vector<Vertex>&&, std::vector<u32>&&);

    template <typename Ty>
    [[nodiscard]] Handle<Ty> upload(const char*);
    void unload(Handle<Mesh>&&);

    void start();
    void draw(const RenderData&);
    void end();
    void submit();
} // namespace tethys::renderer

#endif //TETHYS_RENDERER_HPP
