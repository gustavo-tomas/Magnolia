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
            model->vbo.shutdown();
            model->ibo.shutdown();
        }
    }

    static i32 global_counter = 0;
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
        model->name = scene->GetShortFilename(file.c_str());

        for (u32 m = 0; m < scene->mNumMeshes; m++)
        {
            const aiMesh* mesh = scene->mMeshes[m];
            ASSERT(mesh->HasFaces(), "Mesh has no faces");
            ASSERT(mesh->HasPositions(), "Mesh has no position");
            ASSERT(mesh->HasNormals(), "Mesh has no normals");
            ASSERT(mesh->HasTangentsAndBitangents(), "Mesh has no tangents/bitangents");

            // Material
            const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            // @NOTE: only one texture per mesh
            const str directory = file.substr(0, file.find_last_of('/'));
            if (material->GetTextureCount(aiTextureType_DIFFUSE))
            {
                u32 i = 0;

                aiString ai_mat_name;
                material->GetTexture(aiTextureType_DIFFUSE, i, &ai_mat_name);  // !TODO assert this
                const str material_name = ai_mat_name.C_Str();

                // Textures
                auto texture = Application::get_texture_loader().load(directory + "/" + material_name);
                model->textures.push_back(texture);
                global_counter++;
            }

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

                    // Z component is an index to the texture
                    // @TODO: check for meshes with no textures
                    vertex.tex_coords =
                        vec3(mesh->mTextureCoords[0][idx].x, mesh->mTextureCoords[0][idx].y, global_counter);

                    // vertex.tangent = vec3(mesh->mTangents[idx].x, mesh->mTangents[idx].y, mesh->mTangents[idx].z);
                    // vertex.bitangent =
                    //     vec3(mesh->mBitangents[idx].x, mesh->mBitangents[idx].y, mesh->mBitangents[idx].z);

                    model->vertices.push_back(vertex);
                    model->indices.push_back(VECSIZE(model->indices));  // !TODO: this is not efficient
                }
            }
        }

        // Buffers
        model->vbo.initialize(model->vertices.data(), model->vertices.size() * sizeof(Vertex));
        model->ibo.initialize(model->indices.data(), model->indices.size() * sizeof(u32));

        models[file] = std::shared_ptr<Model>(model);
        return models[file];
    }

    void Cube::initialize()
    {
        model.name = "Cube";
        model.vertices.resize(24);

        // Positions for each vertex
        model.vertices[0].position = {-1.0f, -1.0f, 1.0f};  // Front bottom-left
        model.vertices[1].position = {1.0f, -1.0f, 1.0f};   // Front bottom-right
        model.vertices[2].position = {1.0f, 1.0f, 1.0f};    // Front top-right
        model.vertices[3].position = {-1.0f, 1.0f, 1.0f};   // Front top-left

        model.vertices[4].position = {-1.0f, -1.0f, -1.0f};  // Back bottom-left
        model.vertices[5].position = {1.0f, -1.0f, -1.0f};   // Back bottom-right
        model.vertices[6].position = {1.0f, 1.0f, -1.0f};    // Back top-right
        model.vertices[7].position = {-1.0f, 1.0f, -1.0f};   // Back top-left

        model.vertices[8].position = {-1.0f, -1.0f, 1.0f};   // Left bottom-front
        model.vertices[9].position = {-1.0f, -1.0f, -1.0f};  // Left bottom-back
        model.vertices[10].position = {-1.0f, 1.0f, -1.0f};  // Left top-back
        model.vertices[11].position = {-1.0f, 1.0f, 1.0f};   // Left top-front

        model.vertices[12].position = {1.0f, -1.0f, 1.0f};   // Right bottom-front
        model.vertices[13].position = {1.0f, -1.0f, -1.0f};  // Right bottom-back
        model.vertices[14].position = {1.0f, 1.0f, -1.0f};   // Right top-back
        model.vertices[15].position = {1.0f, 1.0f, 1.0f};    // Right top-front

        model.vertices[16].position = {-1.0f, 1.0f, 1.0f};   // Top front-left
        model.vertices[17].position = {1.0f, 1.0f, 1.0f};    // Top front-right
        model.vertices[18].position = {1.0f, 1.0f, -1.0f};   // Top back-right
        model.vertices[19].position = {-1.0f, 1.0f, -1.0f};  // Top back-left

        model.vertices[20].position = {-1.0f, -1.0f, 1.0f};   // Bottom front-left
        model.vertices[21].position = {1.0f, -1.0f, 1.0f};    // Bottom front-right
        model.vertices[22].position = {1.0f, -1.0f, -1.0f};   // Bottom back-right
        model.vertices[23].position = {-1.0f, -1.0f, -1.0f};  // Bottom back-left

        for (auto& vertex : model.vertices) vertex.normal = normalize(vertex.position);

        // Front face
        model.vertices[0].tex_coords = {0.0f, 0.0f, global_counter};
        model.vertices[1].tex_coords = {1.0f, 0.0f, global_counter};
        model.vertices[2].tex_coords = {1.0f, 1.0f, global_counter};
        model.vertices[3].tex_coords = {0.0f, 1.0f, global_counter};

        // Back face
        model.vertices[4].tex_coords = {0.0f, 0.0f, global_counter};
        model.vertices[5].tex_coords = {1.0f, 0.0f, global_counter};
        model.vertices[6].tex_coords = {1.0f, 1.0f, global_counter};
        model.vertices[7].tex_coords = {0.0f, 1.0f, global_counter};

        // Left face
        model.vertices[8].tex_coords = {0.0f, 0.0f, global_counter};
        model.vertices[9].tex_coords = {1.0f, 0.0f, global_counter};
        model.vertices[10].tex_coords = {1.0f, 1.0f, global_counter};
        model.vertices[11].tex_coords = {0.0f, 1.0f, global_counter};

        // Right face
        model.vertices[12].tex_coords = {0.0f, 0.0f, global_counter};
        model.vertices[13].tex_coords = {1.0f, 0.0f, global_counter};
        model.vertices[14].tex_coords = {1.0f, 1.0f, global_counter};
        model.vertices[15].tex_coords = {0.0f, 1.0f, global_counter};

        // Top face
        model.vertices[16].tex_coords = {0.0f, 0.0f, global_counter};
        model.vertices[17].tex_coords = {1.0f, 0.0f, global_counter};
        model.vertices[18].tex_coords = {1.0f, 1.0f, global_counter};
        model.vertices[19].tex_coords = {0.0f, 1.0f, global_counter};

        // Bottom face
        model.vertices[20].tex_coords = {0.0f, 0.0f, global_counter};
        model.vertices[21].tex_coords = {1.0f, 0.0f, global_counter};
        model.vertices[22].tex_coords = {1.0f, 1.0f, global_counter};
        model.vertices[23].tex_coords = {0.0f, 1.0f, global_counter};

        // Indices for the cube
        model.indices = {// Front face
                         0, 1, 2, 2, 3, 0,
                         // Back face
                         4, 5, 6, 6, 7, 4,
                         // Left face
                         8, 9, 10, 10, 11, 8,
                         // Right face
                         12, 13, 14, 14, 15, 12,
                         // Top face
                         16, 17, 18, 18, 19, 16,
                         // Bottom face
                         20, 21, 22, 22, 23, 20};

        model.vbo.initialize(model.vertices.data(), VECSIZE(model.vertices) * sizeof(Vertex));
        model.ibo.initialize(model.indices.data(), VECSIZE(model.indices) * sizeof(u32));

        // Create a diffuse texture
        auto diffuse_texture = Application::get_texture_loader().load("assets/images/DefaultAlbedoSeamless.png");

        model.textures.push_back(diffuse_texture);
    }

    void Cube::shutdown()
    {
        get_context().get_device().waitIdle();

        model.vbo.shutdown();
        model.ibo.shutdown();
    }
};  // namespace mag
