#ifndef TETHYS_MODEL_HPP
#define TETHYS_MODEL_HPP

#include <tethys/forwards.hpp>
#include <tethys/handle.hpp>
#include <tethys/types.hpp>
#include <tethys/mesh.hpp>

#include <vector>
#include <filesystem>

namespace tethys {
    struct Model {
        struct SubMesh {
            Handle<Mesh> mesh;

            Handle<Texture> diffuse;
            Handle<Texture> specular;
            Handle<Texture> normal;
        };

        std::vector<SubMesh> submeshes;
    };

    [[nodiscard]] Model load_model(const std::filesystem::path&);
    [[nodiscard]] Model load_model(const std::vector<Vertex>&, const std::vector<u32>&, const char*, const char*, const char*);
} // namespace tethys

#endif //TETHYS_MODEL_HPP
