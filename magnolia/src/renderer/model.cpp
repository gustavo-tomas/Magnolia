#include "renderer/model.hpp"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "core/application.hpp"
#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "renderer/image.hpp"

namespace mag
{
    void ModelLoader::initialize() { importer = std::make_unique<Assimp::Importer>(); }

    void ModelLoader::shutdown()
    {
        // @TODO: idk about this
        get_context().get_device().waitIdle();

        for (auto& [name, model] : models)
        {
            for (auto& mesh : model->meshes)
            {
                mesh.ibo.shutdown();
                mesh.vbo.shutdown();
            }
        }
    }

    std::shared_ptr<Model> ModelLoader::load(const str& file)
    {
        auto it = models.find(file);
        if (it != models.end()) return it->second;

        const u32 flags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices |
                          aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs;

        const aiScene* scene = importer->ReadFile(file, flags);
        ASSERT(scene && scene->mRootNode && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "Failed to load model");
        ASSERT(scene->HasMeshes(), "Model has no meshes");

        Model* model = new Model();
        for (u32 m = 0; m < scene->mNumMeshes; m++)
        {
            const aiMesh* mesh = scene->mMeshes[m];
            ASSERT(mesh->HasFaces(), "Mesh has no faces");
            ASSERT(mesh->HasPositions(), "Mesh has no position");
            ASSERT(mesh->HasNormals(), "Mesh has no normals");
            ASSERT(mesh->HasTangentsAndBitangents(), "Mesh has no tangents/bitangents");

            std::vector<Vertex> vertices;
            std::vector<u32> indices;
            std::vector<std::shared_ptr<Image>> textures;

            // Vertices/Indices
            for (u32 f = 0; f < mesh->mNumFaces; f++)
            {
                const aiFace& face = mesh->mFaces[f];
                ASSERT(face.mNumIndices == 3, "Face is not a triangle");

                Vertex vertex = {};
                for (u32 i = 0; i < face.mNumIndices; i++)
                {
                    const u32 idx = face.mIndices[i];

                    vertex.position = vec3(mesh->mVertices[idx].x, mesh->mVertices[idx].y, mesh->mVertices[idx].z);
                    vertex.normal = vec3(mesh->mNormals[idx].x, mesh->mNormals[idx].y, mesh->mNormals[idx].z);
                    vertex.tex_coords = vec2(mesh->mTextureCoords[0][idx].x, mesh->mTextureCoords[0][idx].y);
                    // vertex.tangent = vec3(mesh->mTangents[idx].x, mesh->mTangents[idx].y, mesh->mTangents[idx].z);
                    // vertex.bitangent =
                    //     vec3(mesh->mBitangents[idx].x, mesh->mBitangents[idx].y, mesh->mBitangents[idx].z);

                    vertices.push_back(vertex);
                    indices.push_back(VECSIZE(indices));  // !TODO: this is not efficient
                }
            }

            // Material
            const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            const str directory = file.substr(0, file.find_last_of('/'));
            for (u32 i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++)
            {
                aiString ai_mat_name;
                material->GetTexture(aiTextureType_DIFFUSE, i, &ai_mat_name);  // !TODO assert this
                const str material_name = ai_mat_name.C_Str();

                // Textures
                auto texture = Application::get_texture_loader().load(directory + "/" + material_name);
                textures.push_back(texture);
            }

            // Buffers
            VertexBuffer vbo;
            IndexBuffer ibo;

            vbo.initialize(vertices.data(), vertices.size() * sizeof(Vertex));
            ibo.initialize(indices.data(), indices.size() * sizeof(u32));

            Mesh new_mesh = {vbo, ibo, vertices, indices, textures};
            model->meshes.push_back(new_mesh);
        }

        models[file] = std::shared_ptr<Model>(model);
        return models[file];
    }
};  // namespace mag
