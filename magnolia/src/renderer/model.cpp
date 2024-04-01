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

        for (const auto& model_pair : models)
        {
            const auto& model = model_pair.second;
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
        model->name = scene->GetShortFilename(file.c_str());

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
                auto texture = get_application().get_texture_loader().load(directory + "/" + material_name);
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

    void Cube::initialize()
    {
        model.name = "Cube";
        model.meshes.resize(1);

        auto& mesh = model.meshes[0];
        mesh.vertices.resize(24);

        // Positions for each vertex
        mesh.vertices[0].position = {-1.0f, -1.0f, 1.0f};  // Front bottom-left
        mesh.vertices[1].position = {1.0f, -1.0f, 1.0f};   // Front bottom-right
        mesh.vertices[2].position = {1.0f, 1.0f, 1.0f};    // Front top-right
        mesh.vertices[3].position = {-1.0f, 1.0f, 1.0f};   // Front top-left

        mesh.vertices[4].position = {-1.0f, -1.0f, -1.0f};  // Back bottom-left
        mesh.vertices[5].position = {1.0f, -1.0f, -1.0f};   // Back bottom-right
        mesh.vertices[6].position = {1.0f, 1.0f, -1.0f};    // Back top-right
        mesh.vertices[7].position = {-1.0f, 1.0f, -1.0f};   // Back top-left

        mesh.vertices[8].position = {-1.0f, -1.0f, 1.0f};   // Left bottom-front
        mesh.vertices[9].position = {-1.0f, -1.0f, -1.0f};  // Left bottom-back
        mesh.vertices[10].position = {-1.0f, 1.0f, -1.0f};  // Left top-back
        mesh.vertices[11].position = {-1.0f, 1.0f, 1.0f};   // Left top-front

        mesh.vertices[12].position = {1.0f, -1.0f, 1.0f};   // Right bottom-front
        mesh.vertices[13].position = {1.0f, -1.0f, -1.0f};  // Right bottom-back
        mesh.vertices[14].position = {1.0f, 1.0f, -1.0f};   // Right top-back
        mesh.vertices[15].position = {1.0f, 1.0f, 1.0f};    // Right top-front

        mesh.vertices[16].position = {-1.0f, 1.0f, 1.0f};   // Top front-left
        mesh.vertices[17].position = {1.0f, 1.0f, 1.0f};    // Top front-right
        mesh.vertices[18].position = {1.0f, 1.0f, -1.0f};   // Top back-right
        mesh.vertices[19].position = {-1.0f, 1.0f, -1.0f};  // Top back-left

        mesh.vertices[20].position = {-1.0f, -1.0f, 1.0f};   // Bottom front-left
        mesh.vertices[21].position = {1.0f, -1.0f, 1.0f};    // Bottom front-right
        mesh.vertices[22].position = {1.0f, -1.0f, -1.0f};   // Bottom back-right
        mesh.vertices[23].position = {-1.0f, -1.0f, -1.0f};  // Bottom back-left

        for (auto& vertex : mesh.vertices) vertex.normal = normalize(vertex.position);

        // Front face
        mesh.vertices[0].tex_coords = {0.0f, 0.0f};
        mesh.vertices[1].tex_coords = {1.0f, 0.0f};
        mesh.vertices[2].tex_coords = {1.0f, 1.0f};
        mesh.vertices[3].tex_coords = {0.0f, 1.0f};

        // Back face
        mesh.vertices[4].tex_coords = {0.0f, 0.0f};
        mesh.vertices[5].tex_coords = {1.0f, 0.0f};
        mesh.vertices[6].tex_coords = {1.0f, 1.0f};
        mesh.vertices[7].tex_coords = {0.0f, 1.0f};

        // Left face
        mesh.vertices[8].tex_coords = {0.0f, 0.0f};
        mesh.vertices[9].tex_coords = {1.0f, 0.0f};
        mesh.vertices[10].tex_coords = {1.0f, 1.0f};
        mesh.vertices[11].tex_coords = {0.0f, 1.0f};

        // Right face
        mesh.vertices[12].tex_coords = {0.0f, 0.0f};
        mesh.vertices[13].tex_coords = {1.0f, 0.0f};
        mesh.vertices[14].tex_coords = {1.0f, 1.0f};
        mesh.vertices[15].tex_coords = {0.0f, 1.0f};

        // Top face
        mesh.vertices[16].tex_coords = {0.0f, 0.0f};
        mesh.vertices[17].tex_coords = {1.0f, 0.0f};
        mesh.vertices[18].tex_coords = {1.0f, 1.0f};
        mesh.vertices[19].tex_coords = {0.0f, 1.0f};

        // Bottom face
        mesh.vertices[20].tex_coords = {0.0f, 0.0f};
        mesh.vertices[21].tex_coords = {1.0f, 0.0f};
        mesh.vertices[22].tex_coords = {1.0f, 1.0f};
        mesh.vertices[23].tex_coords = {0.0f, 1.0f};

        // Indices for the cube
        mesh.indices = {// Front face
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

        mesh.vbo.initialize(mesh.vertices.data(), VECSIZE(mesh.vertices) * sizeof(Vertex));
        mesh.ibo.initialize(mesh.indices.data(), VECSIZE(mesh.indices) * sizeof(u32));

        // Create a diffuse texture
        auto diffuse_texture = get_application().get_texture_loader().load("assets/images/DefaultAlbedoSeamless.png");

        mesh.textures.push_back(diffuse_texture);
    }

    void Cube::shutdown()
    {
        get_context().get_device().waitIdle();

        model.meshes[0].vbo.shutdown();
        model.meshes[0].ibo.shutdown();
    }
};  // namespace mag
