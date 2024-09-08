#include "resources/model_loader.hpp"

#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "core/application.hpp"
#include "core/assert.hpp"
#include "core/logger.hpp"
#include "meshoptimizer.h"
#include "resources/material.hpp"

namespace mag
{
#define MATERIAL_FILE_EXTENSION ".mat.json"
#define MODEL_FILE_EXTENSION ".model.json"
#define BINARY_FILE_EXTENSION ".model.bin"

    ModelLoader::ModelLoader() : importer(new Assimp::Importer()) {}

    b8 ModelLoader::load(const str& file_path, Model* model)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        const str extension = file_system.get_file_extension(file_path);

        if (!is_extension_supported(extension))
        {
            LOG_ERROR("Extension '{0}' not supported", extension);
            return false;
        }

        b8 result = false;

        // Reset model data
        *model = {};

        if (extension == ".json")
        {
            result = load_native(file_path, model);
            if (!result)
            {
                LOG_ERROR("Failed to load model: {0}", file_path);
            }
        }

        else
        {
            result = import_from_file(file_path, model);
            if (!result)
            {
                LOG_ERROR("Failed to import model: '{0}'", file_path);
            }
        }

        if (result)
        {
            LOG_SUCCESS("Loaded model: {0}", file_path);
        }

        return result;
    }

    b8 ModelLoader::load_native(const str& file_path, Model* model)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        json data;

        if (!file_system.read_json_data(file_path, data))
        {
            LOG_ERROR("Failed to load native model file: '{0}'", file_path);
            return false;
        }

        if (!data.contains("Model") || !data.contains("File") || !data.contains("Materials"))
        {
            LOG_ERROR("Model file '{0}' has incomplete fields", file_path);
            return false;
        }

        const str model_name = data["Model"];
        const str binary_file_path = data["File"];
        const std::vector<str> materials = data["Materials"];

        Buffer buffer;
        const b8 result = file_system.read_binary_data(binary_file_path, buffer);

        if (!result)
        {
            LOG_ERROR("Failed to load native model binary file: '{0}'", binary_file_path);
            return false;
        }

        // Extract juicy model data
        model->name = model_name;
        model->file_path = file_path;
        model->materials = materials;

        char* model_data = buffer.cast<char>();

        // Read number of vertices
        const u32 num_vertices = *reinterpret_cast<u32*>(model_data);
        model_data += sizeof(u32);

        // Read vertices
        model->vertices.resize(num_vertices);
        memcpy(model->vertices.data(), model_data, VEC_SIZE_BYTES(model->vertices));
        model_data += VEC_SIZE_BYTES(model->vertices);

        // Read number of indices
        const u32 num_indices = *reinterpret_cast<u32*>(model_data);
        model_data += sizeof(u32);

        // Read indices
        model->indices.resize(num_indices);
        memcpy(model->indices.data(), model_data, VEC_SIZE_BYTES(model->indices));
        model_data += VEC_SIZE_BYTES(model->indices);

        // Read number of meshes
        const u32 num_meshes = *reinterpret_cast<u32*>(model_data);
        model_data += sizeof(u32);

        // Read meshes
        model->meshes.resize(num_meshes);
        memcpy(model->meshes.data(), model_data, VEC_SIZE_BYTES(model->meshes));
        model_data += VEC_SIZE_BYTES(model->meshes);

        return true;
    }

    b8 ModelLoader::import_from_file(const str& file_path, Model* model)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        const u32 flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs;

        const aiScene* scene = importer->ReadFile(file_path, flags);
        if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
        {
            LOG_ERROR("Failed to import model '{0}'", file_path);
            return false;
        }

        if (!scene->HasMeshes())
        {
            LOG_ERROR("Model has no meshes '{0}'", file_path);
            return false;
        }

        const str error = importer->GetErrorString();
        if (!error.empty())
        {
            LOG_ERROR("Importer error: {0}", error);
        }

        model->name = scene->mRootNode->mName.C_Str();
        model->meshes.resize(scene->mNumMeshes);

        for (u32 m = 0; m < scene->mNumMeshes; m++)
        {
            const aiMesh* mesh = scene->mMeshes[m];
            initialize_mesh(m, mesh, model);
        }

        const str output_directory = file_path.substr(0, file_path.find_last_of('/')) + "/native";
        if (!file_system.create_directories(output_directory))
        {
            LOG_ERROR("Failed to create directory: '{0}'", output_directory);
            return false;
        }

        initialize_materials(scene, file_path, output_directory, model);
        create_native_file(output_directory, model);

        return true;
    }

    void ModelLoader::create_native_file(const str& output_directory, Model* model)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        const str native_model_file_path = output_directory + "/" + model->name + MODEL_FILE_EXTENSION;
        const str binary_file_path = output_directory + "/" + model->name + BINARY_FILE_EXTENSION;

        json data;
        data["Model"] = model->name;
        data["File"] = binary_file_path;
        data["Materials"] = model->materials;

        // Write the data to the native file format
        if (!file_system.write_json_data(native_model_file_path, data))
        {
            LOG_ERROR("Failed to create native model file: '{0}'", native_model_file_path);
            return;
        }

        const u32 num_vertices = model->vertices.size();
        const u32 num_indices = model->indices.size();
        const u32 num_meshes = model->meshes.size();

        Buffer buffer;

        u64 buffer_size = 0;
        buffer_size += sizeof(num_vertices) + VEC_SIZE_BYTES(model->vertices);
        buffer_size += sizeof(num_indices) + VEC_SIZE_BYTES(model->indices);
        buffer_size += sizeof(num_meshes) + VEC_SIZE_BYTES(model->meshes);

        buffer.data.resize(buffer_size);

        u8* ptr = buffer.data.data();

        // Write vertices
        memcpy(ptr, &num_vertices, sizeof(num_vertices));
        ptr += sizeof(num_vertices);

        memcpy(ptr, model->vertices.data(), VEC_SIZE_BYTES(model->vertices));
        ptr += VEC_SIZE_BYTES(model->vertices);

        // Write indices
        memcpy(ptr, &num_indices, sizeof(num_indices));
        ptr += sizeof(num_indices);

        memcpy(ptr, model->indices.data(), VEC_SIZE_BYTES(model->indices));
        ptr += VEC_SIZE_BYTES(model->indices);

        // Write meshes
        memcpy(ptr, &num_meshes, sizeof(num_meshes));
        ptr += sizeof(num_meshes);

        memcpy(ptr, model->meshes.data(), VEC_SIZE_BYTES(model->meshes));
        ptr += VEC_SIZE_BYTES(model->meshes);

        // Write binary model data to file
        if (!file_system.write_binary_data(binary_file_path, buffer))
        {
            LOG_ERROR("Failed to create binary model file: '{0}'", binary_file_path);
            return;
        }

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

    void ModelLoader::initialize_materials(const aiScene* ai_scene, const str& file_path, const str& output_directory,
                                           Model* model)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        const str model_directory = file_path.substr(0, file_path.find_last_of('/'));

        model->materials.resize(ai_scene->mNumMaterials);

        for (u32 i = 0; i < ai_scene->mNumMaterials; i++)
        {
            const aiMaterial* ai_material = ai_scene->mMaterials[i];
            str material_name = ai_material->GetName().C_Str();
            str material_file_path = output_directory + "/" + material_name + MATERIAL_FILE_EXTENSION;

            // Invalid material, use the default one instead
            if (material_name.empty())
            {
                material_name = "Default";
                material_file_path = DEFAULT_MATERIAL_NAME;
            }

            model->materials[i] = material_file_path;

            // Write material data to file
            json data;
            data["Material"] = material_name;
            data["Textures"]["Albedo"] = find_texture(ai_material, aiTextureType_DIFFUSE, model_directory);
            data["Textures"]["Normal"] = find_texture(ai_material, aiTextureType_NORMALS, model_directory);

            if (!file_system.write_json_data(material_file_path, data))
            {
                LOG_ERROR("Failed to create material file: {0}", material_file_path);
                continue;
            }
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
