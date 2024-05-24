#include "renderer/model.hpp"

#include <assimp/postprocess.h>

#include "core/application.hpp"
#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "renderer/image.hpp"

namespace mag
{
    ModelManager::ModelManager() { importer = std::make_unique<Assimp::Importer>(); }

    ModelManager::~ModelManager()
    {
        // @TODO: idk about this
        get_context().get_device().waitIdle();

        for (const auto& model_pair : models)
        {
            const auto& model = model_pair.second;
            model->vbo.shutdown();
            model->ibo.shutdown();
        }
    }

    std::shared_ptr<Model> ModelManager::load(const str& file)
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
        model->meshes.resize(scene->mNumMeshes);

        for (u32 m = 0; m < scene->mNumMeshes; m++)
        {
            const aiMesh* mesh = scene->mMeshes[m];
            initialize_mesh(m, mesh, model);
        }

        model->vbo.initialize(model->vertices.data(), model->vertices.size() * sizeof(Vertex));
        model->ibo.initialize(model->indices.data(), model->indices.size() * sizeof(u32));

        initialize_materials(scene, file, model);

        LOG_SUCCESS("Loaded model: {0}", file);
        models[file] = std::shared_ptr<Model>(model);
        return models[file];
    }

    void ModelManager::initialize_mesh(const u32 mesh_idx, const aiMesh* ai_mesh, Model* model)
    {
        ASSERT(ai_mesh->HasFaces(), "Mesh has no faces");
        ASSERT(ai_mesh->HasPositions(), "Mesh has no position");
        ASSERT(ai_mesh->HasNormals(), "Mesh has no normals");
        ASSERT(ai_mesh->HasTangentsAndBitangents(), "Mesh has no tangents/bitangents");

        model->meshes[mesh_idx].base_index = model->indices.size();
        model->meshes[mesh_idx].base_vertex = model->vertices.size();
        model->meshes[mesh_idx].index_count = ai_mesh->mNumFaces * 3;
        model->meshes[mesh_idx].material_index = ai_mesh->mMaterialIndex;

        // Vertices
        for (u32 i = 0; i < ai_mesh->mNumVertices; i++)
        {
            Vertex vertex = {};

            vertex.position = vec3(ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z);
            vertex.normal = vec3(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z);
            vertex.tex_coords = vec2(ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y);

            model->vertices.push_back(vertex);
        }

        // Indices
        for (u32 i = 0; i < ai_mesh->mNumFaces; i++)
        {
            const auto& face = ai_mesh->mFaces[i];
            ASSERT(face.mNumIndices == 3, "Face is not a triangle");

            for (u32 j = 0; j < face.mNumIndices; j++)
            {
                const auto& idx = face.mIndices[j];
                model->indices.push_back(idx);
            }
        }
    }

    void ModelManager::initialize_materials(const aiScene* ai_scene, const str& file, Model* model)
    {
        auto& app = get_application();
        auto& material_loader = app.get_material_loader();

        model->materials.resize(ai_scene->mNumMaterials);

        for (u32 i = 0; i < ai_scene->mNumMaterials; i++)
        {
            const aiMaterial* ai_material = ai_scene->mMaterials[i];

            Material* material = new Material();

            const u32 texture_count = ai_material->GetTextureCount(aiTextureType_DIFFUSE);
            const str directory = file.substr(0, file.find_last_of('/'));

            if (texture_count > 1)
            {
                LOG_ERROR("Only one texture for each mesh is supported");
            }

            for (u32 j = 0; j < texture_count; j++)
            {
                aiString ai_mat_name;
                auto result = ai_material->GetTexture(aiTextureType_DIFFUSE, j, &ai_mat_name);

                if (result != aiReturn::aiReturn_SUCCESS)
                {
                    LOG_ERROR("Failed to retrieve texture with index {0}", j);
                    continue;
                }

                const str texture_path = directory + "/" + ai_mat_name.C_Str();
                material->diffuse_texture = app.get_texture_loader().load(texture_path);
                material->name = ai_material->GetName().C_Str();

                LOG_INFO("Loaded texture: {0}", texture_path);
            }

            // Load default textures if none are found
            if (material->diffuse_texture == nullptr)
            {
                material->diffuse_texture =
                    app.get_texture_loader().load("magnolia/assets/images/DefaultAlbedoSeamless.png");

                material->name = "Default";
            }

            model->materials[i] = material_loader.load(material);
        }
    }

    b8 ModelManager::is_extension_supported(const str& extension_with_dot)
    {
        return importer->IsExtensionSupported(extension_with_dot);
    }

    Cube::Cube(const str& name)
    {
        model.name = name;
        model.meshes.resize(1);
        model.meshes[0].base_index = 0;
        model.meshes[0].base_vertex = 0;
        model.meshes[0].material_index = 0;

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
        model.vertices[0].tex_coords = {0.0f, 0.0f};
        model.vertices[1].tex_coords = {1.0f, 0.0f};
        model.vertices[2].tex_coords = {1.0f, 1.0f};
        model.vertices[3].tex_coords = {0.0f, 1.0f};

        // Back face
        model.vertices[4].tex_coords = {0.0f, 0.0f};
        model.vertices[5].tex_coords = {1.0f, 0.0f};
        model.vertices[6].tex_coords = {1.0f, 1.0f};
        model.vertices[7].tex_coords = {0.0f, 1.0f};

        // Left face
        model.vertices[8].tex_coords = {0.0f, 0.0f};
        model.vertices[9].tex_coords = {1.0f, 0.0f};
        model.vertices[10].tex_coords = {1.0f, 1.0f};
        model.vertices[11].tex_coords = {0.0f, 1.0f};

        // Right face
        model.vertices[12].tex_coords = {0.0f, 0.0f};
        model.vertices[13].tex_coords = {1.0f, 0.0f};
        model.vertices[14].tex_coords = {1.0f, 1.0f};
        model.vertices[15].tex_coords = {0.0f, 1.0f};

        // Top face
        model.vertices[16].tex_coords = {0.0f, 0.0f};
        model.vertices[17].tex_coords = {1.0f, 0.0f};
        model.vertices[18].tex_coords = {1.0f, 1.0f};
        model.vertices[19].tex_coords = {0.0f, 1.0f};

        // Bottom face
        model.vertices[20].tex_coords = {0.0f, 0.0f};
        model.vertices[21].tex_coords = {1.0f, 0.0f};
        model.vertices[22].tex_coords = {1.0f, 1.0f};
        model.vertices[23].tex_coords = {0.0f, 1.0f};

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

        model.meshes[0].index_count = VECSIZE(model.indices);

        model.vbo.initialize(model.vertices.data(), VECSIZE(model.vertices) * sizeof(Vertex));
        model.ibo.initialize(model.indices.data(), VECSIZE(model.indices) * sizeof(u32));

        auto& app = get_application();
        auto& material_loader = app.get_material_loader();
        auto& texture_loader = app.get_texture_loader();

        // Create a diffuse texture
        Material* material = new Material();
        material->diffuse_texture = texture_loader.load("magnolia/assets/images/DefaultAlbedoSeamless.png");
        material->name = "Default";

        model.materials.push_back(material_loader.load(material));
    }

    Cube::~Cube()
    {
        get_context().get_device().waitIdle();

        model.vbo.shutdown();
        model.ibo.shutdown();
    }
};  // namespace mag
