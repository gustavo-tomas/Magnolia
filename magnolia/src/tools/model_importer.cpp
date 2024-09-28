#include "tools/model_importer.hpp"

#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "core/application.hpp"
#include "core/logger.hpp"
#include "meshoptimizer.h"
#include "resources/material.hpp"

namespace mag
{
#define MATERIAL_FILE_EXTENSION ".mat.json"
#define MODEL_FILE_EXTENSION ".model.json"
#define BINARY_FILE_EXTENSION ".model.bin"

    ModelImporter::ModelImporter() : importer(new Assimp::Importer()) {}

    b8 ModelImporter::import(const str& file_path, str& imported_model_path)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        const u32 flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes |
                          aiProcess_PreTransformVertices;

        const aiScene* scene = importer->ReadFile(file_path, flags);
        if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
        {
            LOG_ERROR("Failed to import model '{0}': {1}", file_path, importer->GetErrorString());
            return false;
        }

        if (!scene->HasMeshes())
        {
            LOG_ERROR("Model has no meshes '{0}'", file_path);
            return false;
        }

        Model model = {};
        model.name = scene->mRootNode->mName.C_Str();
        model.meshes.resize(scene->mNumMeshes);

        for (u32 m = 0; m < scene->mNumMeshes; m++)
        {
            const aiMesh* mesh = scene->mMeshes[m];
            if (!initialize_mesh(m, mesh, model))
            {
                return false;
            }
        }

        // Sort meshes by ascending order of material index
        std::sort(model.meshes.begin(), model.meshes.end(),
                  [](const Mesh& a, const Mesh& b) { return a.material_index < b.material_index; });

        const str output_directory = file_path.substr(0, file_path.find_last_of('/')) + "/native";
        if (!file_system.create_directories(output_directory))
        {
            LOG_ERROR("Failed to create directory: '{0}'", output_directory);
            return false;
        }

        initialize_materials(scene, file_path, output_directory, model);
        return create_native_file(output_directory, model, imported_model_path);
    }

    b8 ModelImporter::create_native_file(const str& output_directory, const Model& model, str& imported_model_path)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        const str native_model_file_path = output_directory + "/" + model.name + MODEL_FILE_EXTENSION;
        const str binary_file_path = output_directory + "/" + model.name + BINARY_FILE_EXTENSION;

        json data;
        data["Model"] = model.name;
        data["File"] = binary_file_path;
        data["Materials"] = model.materials;

        // Write the data to the native file format
        if (!file_system.write_json_data(native_model_file_path, data))
        {
            LOG_ERROR("Failed to create native model file: '{0}'", native_model_file_path);
            return false;
        }

        const u32 num_vertices = model.vertices.size();
        const u32 num_indices = model.indices.size();
        const u32 num_meshes = model.meshes.size();

        Buffer buffer;

        u64 buffer_size = 0;
        buffer_size += sizeof(num_vertices) + VEC_SIZE_BYTES(model.vertices);
        buffer_size += sizeof(num_indices) + VEC_SIZE_BYTES(model.indices);
        buffer_size += sizeof(num_meshes) + VEC_SIZE_BYTES(model.meshes);

        buffer.data.resize(buffer_size);

        u8* ptr = buffer.data.data();

        // Write vertices
        memcpy(ptr, &num_vertices, sizeof(num_vertices));
        ptr += sizeof(num_vertices);

        memcpy(ptr, model.vertices.data(), VEC_SIZE_BYTES(model.vertices));
        ptr += VEC_SIZE_BYTES(model.vertices);

        // Write indices
        memcpy(ptr, &num_indices, sizeof(num_indices));
        ptr += sizeof(num_indices);

        memcpy(ptr, model.indices.data(), VEC_SIZE_BYTES(model.indices));
        ptr += VEC_SIZE_BYTES(model.indices);

        // Write meshes
        memcpy(ptr, &num_meshes, sizeof(num_meshes));
        ptr += sizeof(num_meshes);

        memcpy(ptr, model.meshes.data(), VEC_SIZE_BYTES(model.meshes));
        ptr += VEC_SIZE_BYTES(model.meshes);

        // Write binary model data to file
        if (!file_system.write_binary_data(binary_file_path, buffer))
        {
            LOG_ERROR("Failed to create binary model file: '{0}'", binary_file_path);
            return false;
        }

        imported_model_path = native_model_file_path;
        return true;
    }

    b8 ModelImporter::initialize_mesh(const u32 mesh_idx, const aiMesh* ai_mesh, Model& model)
    {
        if (!ai_mesh->HasFaces())
        {
            LOG_ERROR("Mesh has no faces");
            return false;
        }

        if (!ai_mesh->HasPositions())
        {
            LOG_ERROR("Mesh has no position");
            return false;
        }

        if (!ai_mesh->HasTextureCoords(0))
        {
            LOG_ERROR("Mesh has no texture coordinates");
            return false;
        }

        if (!ai_mesh->HasNormals())
        {
            LOG_ERROR("Mesh has no normals");
            return false;
        }

        if (!ai_mesh->HasTangentsAndBitangents())
        {
            LOG_ERROR("Mesh has no tangents/bitangents");
            return false;
        }

        model.meshes[mesh_idx].base_index = model.indices.size();
        model.meshes[mesh_idx].base_vertex = model.vertices.size();
        model.meshes[mesh_idx].index_count = ai_mesh->mNumFaces * 3;
        model.meshes[mesh_idx].material_index = ai_mesh->mMaterialIndex;
        model.meshes[mesh_idx].aabb_min = {ai_mesh->mAABB.mMin.x, ai_mesh->mAABB.mMin.y, ai_mesh->mAABB.mMin.z};
        model.meshes[mesh_idx].aabb_max = {ai_mesh->mAABB.mMax.x, ai_mesh->mAABB.mMax.y, ai_mesh->mAABB.mMax.z};

        std::vector<u32> indices(ai_mesh->mNumFaces * 3);

        // Indices
        for (u32 i = 0; i < ai_mesh->mNumFaces; i++)
        {
            const auto& face = ai_mesh->mFaces[i];
            if (face.mNumIndices != 3)
            {
                LOG_ERROR("Face is not a triangle");
                return false;
            }

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
        return true;
    }

    void ModelImporter::optimize_mesh(std::vector<Vertex>& vertices, std::vector<u32>& indices, Model& model)
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
        model.vertices.insert(model.vertices.end(), optimized_vertices.begin(), optimized_vertices.end());
        model.indices.insert(model.indices.end(), optimized_indices.begin(), optimized_indices.end());
    }

    void ModelImporter::initialize_materials(const aiScene* ai_scene, const str& file_path, const str& output_directory,
                                             Model& model)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        const str model_directory = file_path.substr(0, file_path.find_last_of('/'));

        model.materials.resize(ai_scene->mNumMaterials);

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

            model.materials[i] = material_file_path;

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

    const str ModelImporter::find_texture(const aiMaterial* ai_material, aiTextureType ai_type,
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

    b8 ModelImporter::is_extension_supported(const str& extension_with_dot)
    {
        return importer->IsExtensionSupported(extension_with_dot);
    }
};  // namespace mag
