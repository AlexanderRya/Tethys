#ifndef TETHYS_RENDERER_HPP
#define TETHYS_RENDERER_HPP

#include <tethys/forwards.hpp>
#include <tethys/handle.hpp>
#include <tethys/mesh.hpp>

#include <glm/vec4.hpp>

#include <filesystem>
#include <vector>

namespace tethys::renderer {
    void initialise();

    [[nodiscard]] Handle<Mesh> write_geometry(const std::vector<Vertex>&, const std::vector<u32>&);
    [[nodiscard]] Handle<Texture> upload_texture(const u8, const u8, const u8, const u8, const ColorSpace);
    [[nodiscard]] Handle<Texture> upload_texture(const char*, const ColorSpace);
    [[nodiscard]] Handle<Model> upload_model(const std::string&);
    [[nodiscard]] Handle<Model> upload_model(const std::vector<Vertex>&, const std::vector<u32>&, const char*, const char*, const char*);
    void unload_geometry(Handle<Mesh>&&);

    void start();
    void draw(const RenderData&);
    void end();
    void submit();
} // namespace tethys::renderer

#endif //TETHYS_RENDERER_HPP
