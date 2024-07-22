#include "renderer/model.hpp"

#include "assimp/postprocess.h"
#include "core/application.hpp"
#include "core/logger.hpp"
#include "meshoptimizer.h"
#include "renderer/context.hpp"
#include "renderer/descriptors.hpp"
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

        const u32 flags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals |
                          aiProcess_CalcTangentSpace | aiProcess_FlipUVs;

        const aiScene* scene = importer->ReadFile(file, flags);
        ASSERT(scene && scene->mRootNode && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "Failed to load model");
        ASSERT(scene->HasMeshes(), "Model has no meshes");

        Model* model = new Model();
        model->file_path = file;
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

        std::vector<u32> indices(ai_mesh->mNumFaces * 3);

        // Indices
        for (u32 i = 0; i < ai_mesh->mNumFaces; i++)
        {
            const auto& face = ai_mesh->mFaces[i];
            ASSERT(face.mNumIndices == 3, "Face is not a triangle");

            indices[i * 3 + 0] = face.mIndices[0];
            indices[i * 3 + 1] = face.mIndices[1];
            indices[i * 3 + 2] = face.mIndices[2];
        }

        std::vector<Vertex> vertices(indices.size());

        // Vertices - load with duplicates. The optimization step will create a better vertex/index buffer.
        for (u32 i = 0; i < indices.size(); i++)
        {
            Vertex vertex = {};

            const u32 idx = indices[i];
            vertex.position = {ai_mesh->mVertices[idx].x, ai_mesh->mVertices[idx].y, ai_mesh->mVertices[idx].z};
            vertex.normal = {ai_mesh->mNormals[idx].x, ai_mesh->mNormals[idx].y, ai_mesh->mNormals[idx].z};
            vertex.tex_coords = {ai_mesh->mTextureCoords[0][idx].x, ai_mesh->mTextureCoords[0][idx].y};
            vertex.tangent = {ai_mesh->mTangents[idx].x, ai_mesh->mTangents[idx].y, ai_mesh->mTangents[idx].z};
            vertex.bitangent = {ai_mesh->mBitangents[idx].x, ai_mesh->mBitangents[idx].y, ai_mesh->mBitangents[idx].z};

            vertices[i] = vertex;
        }

        // Optimize
        optimize_mesh(vertices, indices, model);
    }

    void ModelManager::optimize_mesh(std::vector<Vertex>& vertices, std::vector<u32>& indices, Model* model)
    {
        const u32 vertex_count = vertices.size();
        const u32 index_count = indices.size();

        std::vector<u32> remap(index_count);
        const u64 optimized_vertex_count =
            meshopt_generateVertexRemap(remap.data(), NULL, index_count, vertices.data(), vertex_count, sizeof(Vertex));

        std::vector<Vertex> optimized_vertices(optimized_vertex_count);
        std::vector<u32> optimized_indices(index_count);

        // Remove duplicates
        meshopt_remapIndexBuffer(optimized_indices.data(), NULL, index_count, remap.data());
        meshopt_remapVertexBuffer(optimized_vertices.data(), vertices.data(), vertex_count, sizeof(Vertex),
                                  remap.data());

        // Improve vertex locality
        meshopt_optimizeVertexCache(optimized_indices.data(), optimized_indices.data(), index_count,
                                    optimized_vertex_count);

        // Reduce pixel overdraw
        meshopt_optimizeOverdraw(optimized_indices.data(), optimized_indices.data(), index_count,
                                 &(optimized_vertices[0].position.x), optimized_vertex_count, sizeof(Vertex), 1.05f);

        // Optimize vertex buffer access
        meshopt_optimizeVertexFetch(optimized_vertices.data(), optimized_indices.data(), index_count,
                                    optimized_vertices.data(), optimized_vertex_count, sizeof(Vertex));

        // Insert result into array
        model->vertices.insert(model->vertices.end(), optimized_vertices.begin(), optimized_vertices.end());
        model->indices.insert(model->indices.end(), optimized_indices.begin(), optimized_indices.end());
    }

    void ModelManager::initialize_materials(const aiScene* ai_scene, const str& file, Model* model)
    {
        auto& app = get_application();
        auto& material_manager = app.get_material_manager();

        const str directory = file.substr(0, file.find_last_of('/'));

        model->materials.resize(ai_scene->mNumMaterials);

        for (u32 i = 0; i < ai_scene->mNumMaterials; i++)
        {
            const aiMaterial* ai_material = ai_scene->mMaterials[i];
            const str material_name = ai_material->GetName().C_Str();

            if (material_manager.exists(material_name))
            {
                model->materials[i] = material_manager.get(material_name);
                continue;
            }

            Material* material = new Material();

            material->name = ai_material->GetName().C_Str();

            material->textures[Material::TextureSlot::Albedo] =
                load_texture(ai_material, aiTextureType_DIFFUSE, directory);

            material->textures[Material::TextureSlot::Normal] =
                load_texture(ai_material, aiTextureType_NORMALS, directory);

            // @TODO: hardcoded binding (0)
            for (u32 t = 0; t < Material::TextureCount; t++)
            {
                DescriptorBuilder::create_descriptor_for_texture(0, material->textures[t], material->descriptor_sets[t],
                                                                 material->descriptor_set_layouts[t]);
            }

            model->materials[i] = material_manager.load(material);
        }
    }

    std::shared_ptr<Image> ModelManager::load_texture(const aiMaterial* ai_material, aiTextureType ai_type,
                                                      const str& directory)
    {
        auto& app = get_application();
        auto& material_manager = app.get_material_manager();
        auto& texture_manager = app.get_texture_manager();

        const TextureType type = TextureType(ai_type);
        const str material_name = ai_material->GetName().C_Str();

        // For some reason, assimp may identify normal textures as height textures
        u32 texture_count = ai_material->GetTextureCount(ai_type);
        if (ai_type == aiTextureType_NORMALS && texture_count == 0)
        {
            ai_type = aiTextureType_HEIGHT;
            texture_count = ai_material->GetTextureCount(ai_type);
        }

        std::shared_ptr<Image> texture;
        switch (type)
        {
            case TextureType::Albedo:
                texture = material_manager.get(DEFAULT_MATERIAL_NAME)->textures[Material::TextureSlot::Albedo];
                break;

            case TextureType::Normal:
                texture = material_manager.get(DEFAULT_MATERIAL_NAME)->textures[Material::TextureSlot::Normal];
                break;

            default:
                break;
        }

        if (texture_count > 1)
        {
            LOG_ERROR("Only one texture for each mesh is supported");
        }

        // Load the texture
        if (texture_count > 0)
        {
            aiString ai_tex_path;
            auto result = ai_material->GetTexture(ai_type, 0, &ai_tex_path);

            if (result != aiReturn::aiReturn_SUCCESS)
            {
                LOG_ERROR("Failed to retrieve texture with index {0}, using default", 0);
                return texture;
            }

            LOG_INFO("TEX: {0}", ai_tex_path.C_Str());
            const str texture_path = directory + "/" + ai_tex_path.C_Str();
            texture = texture_manager.load(texture_path, type);

            LOG_INFO("Loaded texture: {0}", texture_path);
            return texture;
        }

        // No textures, use default
        LOG_WARNING("Material '{0}' has no texture of type '{1}', using default", material_name,
                    std::to_string(ai_type));
        return texture;
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

        // Tangents and Bitangents
        for (u32 i = 0; i < model.vertices.size(); i += 3)
        {
            Vertex& v0 = model.vertices[i];
            Vertex& v1 = model.vertices[i + 1];
            Vertex& v2 = model.vertices[i + 2];

            const vec3 edge_1 = v1.position - v0.position;
            const vec3 edge_2 = v2.position - v0.position;

            const vec2 delta_uv1 = v1.tex_coords - v0.tex_coords;
            const vec2 delta_uv2 = v2.tex_coords - v0.tex_coords;

            const f32 f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

            vec3 tangent;
            tangent.x = f * (delta_uv2.y * edge_1.x - delta_uv1.y * edge_2.x);
            tangent.y = f * (delta_uv2.y * edge_1.y - delta_uv1.y * edge_2.y);
            tangent.z = f * (delta_uv2.y * edge_1.z - delta_uv1.y * edge_2.z);

            vec3 bitangent;
            bitangent.x = f * (-delta_uv2.x * edge_1.x + delta_uv1.x * edge_2.x);
            bitangent.y = f * (-delta_uv2.x * edge_1.y + delta_uv1.x * edge_2.y);
            bitangent.z = f * (-delta_uv2.x * edge_1.z + delta_uv1.x * edge_2.z);

            v0.tangent = v1.tangent = v2.tangent = normalize(tangent);
            v0.bitangent = v1.bitangent = v2.bitangent = normalize(bitangent);
        }

        // Indices for the cube
        model.indices = {// Front face
                         0, 1, 2, 2, 3, 0,
                         // Back face
                         6, 5, 4, 7, 6, 4,
                         // Left face
                         10, 9, 8, 11, 10, 8,
                         // Right face
                         12, 13, 14, 14, 15, 12,
                         // Top face
                         16, 17, 18, 18, 19, 16,
                         // Bottom face
                         22, 21, 20, 23, 22, 20};

        model.meshes[0].index_count = VECSIZE(model.indices);

        model.vbo.initialize(model.vertices.data(), VECSIZE(model.vertices) * sizeof(Vertex));
        model.ibo.initialize(model.indices.data(), VECSIZE(model.indices) * sizeof(u32));

        auto& app = get_application();
        auto& material_manager = app.get_material_manager();

        // Use the default material
        model.materials.push_back(material_manager.get(DEFAULT_MATERIAL_NAME));
    }

    Cube::~Cube()
    {
        get_context().get_device().waitIdle();

        model.vbo.shutdown();
        model.ibo.shutdown();
    }

    Line::Line(const str& name, const std::vector<vec3>& starts, const std::vector<vec3>& ends,
               const std::vector<vec3>& colors)
    {
        model.name = name;
        model.meshes.resize(1);
        model.meshes[0].base_vertex = 0;
        model.meshes[0].base_index = 0;

        for (u32 i = 0; i < starts.size(); i++)
        {
            Vertex line_start;
            Vertex line_end;

            line_start.position = starts[i];
            line_end.position = ends[i];

            // We use the models normal as the color
            line_start.normal = colors[i];
            line_end.normal = colors[i];

            model.vertices.push_back(line_start);
            model.vertices.push_back(line_end);
        }

        model.vbo.initialize(model.vertices.data(), VECSIZE(model.vertices) * sizeof(Vertex));
    }

    Line::~Line()
    {
        get_context().get_device().waitIdle();

        model.vbo.shutdown();
    }
};  // namespace mag
