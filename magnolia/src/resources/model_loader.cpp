#include "resources/model_loader.hpp"

#include <fstream>

#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "core/application.hpp"
#include "core/logger.hpp"
#include "meshoptimizer.h"
#include "nlohmann/json.hpp"
#include "renderer/material.hpp"
#include "resources/material_loader.hpp"

namespace mag
{
#define MATERIAL_FILE_EXTENSION ".mat.json"
#define MODEL_FILE_EXTENSION ".model.json"
#define BINARY_FILE_EXTENSION ".model.bin"

    using json = nlohmann::ordered_json;

    ModelLoader::ModelLoader() : importer(new Assimp::Importer()) {}

    Model* ModelLoader::load(const str& file_path)
    {
        const std::filesystem::path filesystem_path(file_path);
        const str extension = filesystem_path.extension().c_str();

        if (!is_extension_supported(extension))
        {
            LOG_ERROR("Extension '{0}' not supported", extension);
            return nullptr;
        }

        Model* result = nullptr;

        if (extension == ".json")
        {
            result = load_native(file_path);
        }

        else
        {
            result = import_from_file(file_path);
        }

        if (result)
        {
            LOG_SUCCESS("Loaded model: {0}", file_path);
        }

        else
        {
            LOG_ERROR("Failed to load model: {0}", file_path);
        }

        return result;
    }

    Model* ModelLoader::load_native(const str& file_path)
    {
        std::ifstream file(file_path);

        if (!file.is_open())
        {
            LOG_ERROR("Failed to open model file: '{0}'", file_path);
            return nullptr;
        }

        const json data = json::parse(file);

        if (!data.contains("Model") || !data.contains("File") || !data.contains("Materials"))
        {
            LOG_ERROR("Model file '{0}' has incomplete fields", file_path);
            return nullptr;
        }

        const str model_name = data["Model"];
        const str binary_file_path = data["File"];
        const std::vector<str> materials = data["Materials"];

        std::ifstream binary_file(binary_file_path, std::ios::binary);

        if (!binary_file.is_open())
        {
            LOG_ERROR("Failed to open model binary file: '{0}'", binary_file_path);
            return nullptr;
        }

        // Extract juicy model data
        Model* model = new Model();
        model->name = model_name;
        model->file_path = file_path;
        model->materials = materials;

        // Read number of vertices
        u32 num_vertices = 0;
        binary_file.read(reinterpret_cast<char*>(&num_vertices), sizeof(num_vertices));

        // Read vertices
        model->vertices.resize(num_vertices);
        binary_file.read(reinterpret_cast<char*>(model->vertices.data()), num_vertices * sizeof(model->vertices[0]));

        // Read number of indices
        u32 num_indices = 0;
        binary_file.read(reinterpret_cast<char*>(&num_indices), sizeof(num_indices));

        // Read indices
        model->indices.resize(num_indices);
        binary_file.read(reinterpret_cast<char*>(model->indices.data()), num_indices * sizeof(model->indices[0]));

        // Read number of meshes
        u32 num_meshes = 0;
        binary_file.read(reinterpret_cast<char*>(&num_meshes), sizeof(num_meshes));

        // Read meshes
        model->meshes.resize(num_meshes);
        binary_file.read(reinterpret_cast<char*>(model->meshes.data()), num_meshes * sizeof(model->meshes[0]));

        return model;
    }

    Model* ModelLoader::import_from_file(const str& file_path)
    {
        // Import the model
        const u32 flags = aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals |
                          aiProcess_CalcTangentSpace | aiProcess_FlipUVs;

        const aiScene* scene = importer->ReadFile(file_path, flags);
        ASSERT(scene && scene->mRootNode && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "Failed to load model");
        ASSERT(scene->HasMeshes(), "Model has no meshes");

        Model* model = new Model();
        model->name = scene->mRootNode->mName.C_Str();
        model->meshes.resize(scene->mNumMeshes);

        for (u32 m = 0; m < scene->mNumMeshes; m++)
        {
            const aiMesh* mesh = scene->mMeshes[m];
            initialize_mesh(m, mesh, model);
        }

        initialize_materials(scene, file_path, model);

        const str directory = file_path.substr(0, file_path.find_last_of('/'));
        create_native_file(directory, model);

        return model;
    }

    void ModelLoader::create_native_file(const str& directory, Model* model)
    {
        // Write the data to the native file format
        const str native_model_file_path = directory + "/" + model->name + MODEL_FILE_EXTENSION;

        std::ofstream model_file(native_model_file_path);

        if (!model_file.is_open())
        {
            LOG_ERROR("Failed to create model file: {0}", native_model_file_path);
            return;
        }

        // Write binary model data to file
        const str binary_file_path = directory + "/" + model->name + BINARY_FILE_EXTENSION;

        json data;
        data["Model"] = model->name;
        data["File"] = binary_file_path;
        data["Materials"] = model->materials;

        model_file << std::setw(4) << data;
        model_file.close();

        std::ofstream binary_file(binary_file_path, std::ios::binary);

        if (!binary_file)
        {
            LOG_ERROR("Failed to create binary model file: {0}", binary_file_path);
            return;
        }

        const u32 num_vertices = model->vertices.size();
        const u32 num_indices = model->indices.size();
        const u32 num_meshes = model->meshes.size();

        // Write number of vertices
        binary_file
            .write(reinterpret_cast<const char*>(&num_vertices), sizeof(num_vertices))

            // Write vertices
            .write(reinterpret_cast<const char*>(model->vertices.data()), num_vertices * sizeof(model->vertices[0]));

        // Write number of indices
        binary_file
            .write(reinterpret_cast<const char*>(&num_indices), sizeof(num_indices))

            // Write indices
            .write(reinterpret_cast<const char*>(model->indices.data()), num_indices * sizeof(model->indices[0]));

        // Write number of meshes
        binary_file
            .write(reinterpret_cast<const char*>(&num_meshes), sizeof(num_meshes))

            // Write meshes
            .write(reinterpret_cast<const char*>(model->meshes.data()), num_meshes * sizeof(model->meshes[0]));

        binary_file.close();

        model->file_path = native_model_file_path;
    }

    void ModelLoader::initialize_mesh(const u32 mesh_idx, const aiMesh* ai_mesh, Model* model)
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

    void ModelLoader::optimize_mesh(std::vector<Vertex>& vertices, std::vector<u32>& indices, Model* model)
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

    void ModelLoader::initialize_materials(const aiScene* ai_scene, const str& file_path, Model* model)
    {
        const str directory = file_path.substr(0, file_path.find_last_of('/'));

        model->materials.resize(ai_scene->mNumMaterials);

        for (u32 i = 0; i < ai_scene->mNumMaterials; i++)
        {
            const aiMaterial* ai_material = ai_scene->mMaterials[i];
            str material_name = ai_material->GetName().C_Str();
            str material_file_path = directory + "/" + material_name + MATERIAL_FILE_EXTENSION;

            // Invalid material, use the default one instead
            if (material_name.empty())
            {
                material_name = "Default";
                material_file_path = DEFAULT_MATERIAL_NAME;
            }

            model->materials[i] = material_file_path;

            // Create the material file
            std::ofstream file(material_file_path);

            if (!file.is_open())
            {
                LOG_ERROR("Failed to create material file: {0}", material_file_path);
                return;
            }

            // Write material data to file
            json data;
            data["Material"] = material_name;
            data["Textures"]["Albedo"] = find_texture(ai_material, aiTextureType_DIFFUSE, directory);
            data["Textures"]["Normal"] = find_texture(ai_material, aiTextureType_NORMALS, directory);

            file << std::setw(4) << data;
            file.close();
        }
    }

    const str ModelLoader::find_texture(const aiMaterial* ai_material, aiTextureType ai_type,
                                        const str& directory) const
    {
        auto& app = get_application();
        auto& material_manager = app.get_material_manager();

        const str material_name = ai_material->GetName().C_Str();

        // For some reason, assimp may identify normal textures as height textures
        u32 texture_count = ai_material->GetTextureCount(ai_type);
        if (ai_type == aiTextureType_NORMALS && texture_count == 0)
        {
            ai_type = aiTextureType_HEIGHT;
            texture_count = ai_material->GetTextureCount(ai_type);
        }

        str texture_name = "";
        switch (ai_type)
        {
            case aiTextureType_DIFFUSE:
                texture_name = material_manager.get_default()->textures[TextureSlot::Albedo];
                break;

            case aiTextureType_NORMALS:
            case aiTextureType_HEIGHT:
                texture_name = material_manager.get_default()->textures[TextureSlot::Normal];
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
                return texture_name;
            }

            const str texture_path = directory + "/" + ai_tex_path.C_Str();
            texture_name = texture_path;

            LOG_INFO("Loaded texture: {0}", texture_name);
            return texture_name;
        }

        // No textures, use default
        LOG_WARNING("Material '{0}' has no texture of type '{1}', using default", material_name,
                    std::to_string(ai_type));
        return texture_name;
    }

    b8 ModelLoader::is_extension_supported(const str& extension_with_dot)
    {
        return importer->IsExtensionSupported(extension_with_dot) || (extension_with_dot == ".json");
    }
};  // namespace mag