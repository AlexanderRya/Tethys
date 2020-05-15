#ifndef TETHYS_MODEL_HPP
#define TETHYS_MODEL_HPP

#include <tethys/forwards.hpp>
#include <tethys/texture.hpp>
#include <tethys/handle.hpp>
#include <tethys/types.hpp>
#include <tethys/mesh.hpp>

#include <vector>
#include <filesystem>

namespace tethys {
    struct Model {
        struct SubMesh {
            Mesh mesh;

            Texture albedo;
            Texture metallic;
            Texture normal;
            Texture roughness;
            Texture occlusion;
        };

        std::vector<SubMesh> submeshes;
    };

    [[nodiscard]] Model load_model(const std::string&);
    [[nodiscard]] Model load_model_pbr(const std::string&);
    [[nodiscard]] Model load_model(const VertexData&, const char*, const char*, const char*);
    [[nodiscard]] Model load_model(const VertexData&, const char*, const char*, const char*, const char*, const char*);
} // namespace tethys

#endif //TETHYS_MODEL_HPP
