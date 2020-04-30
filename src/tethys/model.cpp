#include <tethys/renderer/renderer.hpp>
#include <tethys/constants.hpp>
#include <tethys/model.hpp>

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <vulkan/vulkan.hpp>

#include <unordered_map>

namespace tethys {
    static std::unordered_map<std::string, Handle<Texture>> loaded_textures;

    static Handle<Texture> try_load_texture(const aiMaterial* material, const aiTextureType type, const std::string& model_path) {
        using namespace std::string_literals;

        if (!material->GetTextureCount(type)) {
            return type == aiTextureType_DIFFUSE ? texture::white : texture::black;
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
            aiFace& face = mesh->mFaces[i];
            for (usize j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        auto& material = scene->mMaterials[mesh->mMaterialIndex];

        sub_mesh.mesh = renderer::write_geometry(geometry, indices);
        sub_mesh.diffuse = try_load_texture(material, aiTextureType_DIFFUSE, model_path);
        sub_mesh.specular = try_load_texture(material, aiTextureType_SPECULAR, model_path);
        sub_mesh.normal = try_load_texture(material, aiTextureType_HEIGHT, model_path);

        return sub_mesh;
    }

    static void process_node(const aiScene* scene, const aiNode* node, std::vector<Model::SubMesh>& meshes, const std::string& model_path) {
        for (usize i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(load_mesh(scene, mesh, model_path));
        }

        for (usize i = 0; i < node->mNumChildren; i++) {
            process_node(scene, node->mChildren[i], meshes, model_path);
        }
    }

    Model load_model(const std::string& path) {
        Model model;
        Assimp::Importer importer;

        auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error("Failed to load model");
        }

        process_node(scene, scene->mRootNode, model.submeshes, path.substr(0, path.find_last_of('/')));

        return model;
    }

    Model load_model(const std::vector<Vertex>& vertices, const std::vector<u32>& indices, const char* diffuse, const char* specular, const char* normal) {
        Model::SubMesh submesh{}; {
            submesh.mesh = renderer::write_geometry(vertices, indices);
            submesh.diffuse = diffuse ? loaded_textures.find(diffuse) != loaded_textures.end() ? loaded_textures[diffuse] : renderer::upload_texture(diffuse, vk::Format::eR8G8B8A8Srgb) : texture::white;
            submesh.specular = specular ? loaded_textures.find(specular) != loaded_textures.end() ? loaded_textures[specular] : renderer::upload_texture(specular, vk::Format::eR8G8B8A8Unorm) : texture::black;
            submesh.normal = normal ? loaded_textures.find(normal) != loaded_textures.end() ? loaded_textures[normal] : renderer::upload_texture(normal, vk::Format::eR8G8B8A8Unorm) : texture::black;
        }

        return Model{ {
            submesh
        } };
    }
} // namespace tethys