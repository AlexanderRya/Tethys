#include <tethys/renderer/renderer.hpp>
#include <tethys/constants.hpp>
#include <tethys/model.hpp>

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

namespace tethys {
    static Handle<Texture> try_load_texture(const aiMaterial* material, const aiTextureType type) {
        using namespace std::string_literals;
        if (!material->GetTextureCount(type)) {
            return Handle<Texture>{ texture::white };
        }

        aiString str;
        material->GetTexture(type, 0, &str);
        return renderer::upload<Texture>(("../resources/models/nanosuit/"s + str.C_Str()).c_str()); // Temporary lol
    }

    static Model::SubMesh load_mesh(const aiScene* scene, const aiMesh* mesh) {
        Model::SubMesh sub_mesh{};
        std::vector<Vertex> geometry{};
        std::vector<u32> indices{};

        geometry.reserve(mesh->mNumVertices);
        for (usize i = 0; i < mesh->mNumVertices; ++i) {
            Vertex vertex{};

            vertex.pos.x = mesh->mVertices[i].x;
            vertex.pos.x = mesh->mVertices[i].y;
            vertex.pos.x = mesh->mVertices[i].z;

            vertex.norms.x = mesh->mNormals[i].x;
            vertex.norms.x = mesh->mNormals[i].y;
            vertex.norms.x = mesh->mNormals[i].z;

            if (*mesh->mTextureCoords) {
                vertex.uvs.x = (*mesh->mTextureCoords)[i].x;
                vertex.uvs.x = (*mesh->mTextureCoords)[i].y;
            }

            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;

            vertex.bitangent.x = mesh->mBitangents[i].x;
            vertex.bitangent.y = mesh->mBitangents[i].y;
            vertex.bitangent.z = mesh->mBitangents[i].z;

            geometry.emplace_back(vertex);
        }

        for (usize i = 0; i < mesh->mNumFaces; i++) {
            aiFace& face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        auto& material = scene->mMaterials[mesh->mMaterialIndex];

        sub_mesh.mesh = renderer::upload<Mesh>(std::move(geometry), std::move(indices));
        sub_mesh.diffuse = try_load_texture(material, aiTextureType_DIFFUSE);
        sub_mesh.specular = try_load_texture(material, aiTextureType_SPECULAR);
        sub_mesh.normal = try_load_texture(material, aiTextureType_HEIGHT);

        return sub_mesh;
    }

    static std::vector<Model::SubMesh> process_node(const aiScene* scene, const aiNode* node) {
        std::vector<Model::SubMesh> meshes;

        for(unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(load_mesh(scene, mesh));
        }

        for(unsigned int i = 0; i < node->mNumChildren; i++) {
            process_node(scene, node->mChildren[i]);
        }

        return meshes;
    }

    Model load_model(const char* path) {
        Model model;
        Assimp::Importer importer;

        auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error("Failed to load model");
        }

        model.submeshes = process_node(scene, scene->mRootNode);

        return model;
    }
} // namespace tethys