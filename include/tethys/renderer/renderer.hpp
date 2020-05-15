#ifndef TETHYS_RENDERER_HPP
#define TETHYS_RENDERER_HPP

#include <tethys/forwards.hpp>
#include <tethys/handle.hpp>
#include <tethys/mesh.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <filesystem>
#include <vector>

namespace tethys {
    namespace renderer {
        void initialise();

        [[nodiscard]] Mesh write_geometry(const VertexData&);
        [[nodiscard]] Mesh write_geometry(const std::vector<Vertex>&, const std::vector<u32>&);
        [[nodiscard]] Texture upload_texture(const u8, const u8, const u8, const u8, const vk::Format);
        [[nodiscard]] Texture upload_texture(const char*, const vk::Format);
        [[nodiscard]] Model upload_model(const std::string&);
        [[nodiscard]] Model upload_model_pbr(const std::string&);
        [[nodiscard]] Model upload_model(const VertexData&, const char* = nullptr, const char* = nullptr, const char* = nullptr);
        [[nodiscard]] Model upload_model(const VertexData&, const char* = nullptr, const char* = nullptr, const char* = nullptr, const char* = nullptr, const char* = nullptr);

        void draw(const RenderData&);
        void submit();
    } // namespace tethys::renderer

    namespace texture {
        template <usize Nx>
        Texture& get();
    } // namespace tethys::texture

    namespace shader {
        template <usize Nx>
        Pipeline& get();
    } // namespace tethys::shader
} // namespace tethys

#endif //TETHYS_RENDERER_HPP
