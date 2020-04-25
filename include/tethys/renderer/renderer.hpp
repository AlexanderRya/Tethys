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
    [[nodiscard]] Handle<Texture> upload_texture(const u8, const u8, const u8, const u8, const vk::Format);
    [[nodiscard]] Handle<Texture> upload_texture(const char*, const vk::Format);
    [[nodiscard]] Handle<Model> upload_model(const std::string&);
    [[nodiscard]] Handle<Model> upload_model(const std::vector<Vertex>&, const std::vector<u32>&, const char* = nullptr, const char* = nullptr, const char* = nullptr);
    void unload_geometry(Handle<Mesh>&&);

    void start();
    void draw(const RenderData&);
    void end();
    void submit();
} // namespace tethys::renderer

#endif //TETHYS_RENDERER_HPP
