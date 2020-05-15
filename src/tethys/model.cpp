#include <tethys/renderer/renderer.hpp>
#include <tethys/constants.hpp>
#include <tethys/model.hpp>

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <vulkan/vulkan.hpp>

#include <unordered_map>

namespace tethys {
    static std::unordered_map<std::string, Texture> loaded_textures;

    [[nodiscard]] static Texture& try_load_texture(const aiMaterial* material, const aiTextureType type, const std::string& model_path) {
        if (!material->GetTextureCount(type)) {
            switch (type) {
                case aiTextureType_DIFFUSE:
                    return texture::get<texture::white>();
                case aiTextureType_HEIGHT:
                    return texture::get<texture::green>();
                default:
                    return texture::get<texture::black>();
            }
        }

        aiString str;
        material->GetTexture(type, 0, &str);
        std::string path = model_path + "/" + str.C_Str();

        if (loaded_textures.find(path) != loaded_textures.end()) {
            return loaded_textures[path];
        }

        return loaded_textures[path] = renderer::upload_texture(path.c_str(), type == aiTextureType_DIFFUSE ? vk::Format::eR8G8B8A8Srgb : vk::Format::eR8G8B8A8Unorm);
    }

    static Model::SubMesh load_mesh(const aiScene* scene, const aiMesh* mesh, const std::string& model_path) {
        Model::SubMesh sub_mesh{};
        std::vector<Vertex> geometry{};
        std::vector<u32> indices{};

        geometry.reserve(mesh->mNumVertices);
        for (usize i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex{};

            vertex.pos.x = mesh->mVertices[i].x;
            vertex.pos.y = mesh->mVertices[i].y;
            vertex.pos.z = mesh->mVertices[i].z;

            vertex.norms.x = mesh->mNormals[i].x;
            vertex.norms.y = mesh->mNormals[i].y;
            vertex.norms.z = mesh->mNormals[i].z;

            if (mesh->mTextureCoords[0]) {
                vertex.uvs.x = mesh->mTextureCoords[0][i].x;
                vertex.uvs.y = mesh->mTextureCoords[0][i].y;
            }

            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;

            vertex.bitangent.x = mesh->mBitangents[i].x;
            vertex.bitangent.y = mesh->mBitangents[i].y;
            vertex.bitangent.z = mesh->mBitangents[i].z;

            geometry.emplace_back(vertex);
        }

        indices.reserve(mesh->mNumFaces * 3);

        for (usize i = 0; i < mesh->mNumFaces; i++) {
            auto& face = mesh->mFaces[i];
            for (usize j = 0; j < face.mNumIndices; j++) {
                indices.emplace_back(face.mIndices[j]);
            }
        }

        auto& material = scene->mMaterials[mesh->mMaterialIndex];

        sub_mesh.mesh = renderer::write_geometry(geometry, indices);
        sub_mesh.albedo = try_load_texture(material, aiTextureType_DIFFUSE, model_path);
        sub_mesh.metallic = try_load_texture(material, aiTextureType_SPECULAR, model_path);
        sub_mesh.normal = try_load_texture(material, aiTextureType_HEIGHT, model_path);

        return sub_mesh;
    }

    static void process_node(const aiScene* scene, const aiNode* node, std::vector<Model::SubMesh>& meshes, const std::string& model_path) {
        for (usize i = 0; i < node->mNumMeshes; i++) {
            auto mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.emplace_back(load_mesh(scene, mesh, model_path));
        }

        for (usize i = 0; i < node->mNumChildren; i++) {
            process_node(scene, node->mChildren[i], meshes, model_path);
        }
    }

    Model load_model(const std::string& path) {
        Model model;
        Assimp::Importer importer;

        auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error("Failed to load model");
        }

        process_node(scene, scene->mRootNode, model.submeshes, path.substr(0, path.find_last_of('/')));

        return model;
    }

    static Model::SubMesh load_mesh_pbr(const aiMesh* mesh) {
        Model::SubMesh sub_mesh{};
        std::vector<Vertex> geometry{};
        std::vector<u32> indices{};

        geometry.reserve(mesh->mNumVertices);
        for (usize i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex{};

            vertex.pos.x = mesh->mVertices[i].x;
            vertex.pos.y = mesh->mVertices[i].y;
            vertex.pos.z = mesh->mVertices[i].z;

            vertex.norms.x = mesh->mNormals[i].x;
            vertex.norms.y = mesh->mNormals[i].y;
            vertex.norms.z = mesh->mNormals[i].z;

            if (mesh->mTextureCoords[0]) {
                vertex.uvs.x = mesh->mTextureCoords[0][i].x;
                vertex.uvs.y = mesh->mTextureCoords[0][i].y;
            }

            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;

            vertex.bitangent.x = mesh->mBitangents[i].x;
            vertex.bitangent.y = mesh->mBitangents[i].y;
            vertex.bitangent.z = mesh->mBitangents[i].z;

            geometry.emplace_back(vertex);
        }

        indices.reserve(mesh->mNumFaces * 3);

        for (usize i = 0; i < mesh->mNumFaces; i++) {
            auto& face = mesh->mFaces[i];
            for (usize j = 0; j < face.mNumIndices; j++) {
                indices.emplace_back(face.mIndices[j]);
            }
        }

        sub_mesh.mesh = renderer::write_geometry(geometry, indices);

        return sub_mesh;
    }

    static void process_node_pbr(const aiScene* scene, const aiNode* node, std::vector<Model::SubMesh>& meshes) {
        for (usize i = 0; i < node->mNumMeshes; i++) {
            auto mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.emplace_back(load_mesh_pbr(mesh));
        }

        for (usize i = 0; i < node->mNumChildren; i++) {
            process_node_pbr(scene, node->mChildren[i], meshes);
        }
    }

    [[nodiscard]] Model load_model_pbr(const std::string& path) {
        Model model;
        Assimp::Importer importer;

        auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error("Failed to load model");
        }

        process_node_pbr(scene, scene->mRootNode, model.submeshes);

        return model;
    }

    Model load_model(const VertexData& data, const char* albedo, const char* metallic, const char* normal) {
        Model::SubMesh submesh{}; {
            submesh.mesh = renderer::write_geometry(data);
            submesh.albedo = albedo ? loaded_textures.find(albedo) != loaded_textures.end() ? loaded_textures[albedo] : renderer::upload_texture(albedo, vk::Format::eR8G8B8A8Srgb) : texture::get<texture::white>();
            submesh.metallic = metallic ? loaded_textures.find(metallic) != loaded_textures.end() ? loaded_textures[metallic] : renderer::upload_texture(metallic, vk::Format::eR8G8B8A8Unorm) : texture::get<texture::black>();
            submesh.normal = normal ? loaded_textures.find(normal) != loaded_textures.end() ? loaded_textures[normal] : renderer::upload_texture(normal, vk::Format::eR8G8B8A8Unorm) : texture::get<texture::green>();
        }

        return Model{ {
            submesh
        } };
    }

    Model load_model(const VertexData& data, const char* albedo, const char* metallic, const char* normal, const char* roughness, const char* occlusion) {
        Model::SubMesh submesh{}; {
            submesh.mesh = renderer::write_geometry(data);
            submesh.albedo = albedo ? loaded_textures.find(albedo) != loaded_textures.end() ? loaded_textures[albedo] : renderer::upload_texture(albedo, vk::Format::eR8G8B8A8Srgb) : texture::get<texture::white>();
            submesh.metallic = metallic ? loaded_textures.find(metallic) != loaded_textures.end() ? loaded_textures[metallic] : renderer::upload_texture(metallic, vk::Format::eR8G8B8A8Unorm) : texture::get<texture::black>();
            submesh.normal = normal ? loaded_textures.find(normal) != loaded_textures.end() ? loaded_textures[normal] : renderer::upload_texture(normal, vk::Format::eR8G8B8A8Unorm) : texture::get<texture::green>();
            submesh.roughness = roughness ? loaded_textures.find(roughness) != loaded_textures.end() ? loaded_textures[roughness] : renderer::upload_texture(roughness, vk::Format::eR8G8B8A8Unorm) : texture::get<texture::black>();
            submesh.occlusion = occlusion ? loaded_textures.find(occlusion) != loaded_textures.end() ? loaded_textures[occlusion] : renderer::upload_texture(occlusion, vk::Format::eR8G8B8A8Unorm) : texture::get<texture::black>();
        }

        return Model{ {
            submesh
        } };
    }
} // namespace tethys