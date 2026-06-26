#include "magnolia/tools/model_importer.hpp"

#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "magnolia/core/buffer.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"
#include "magnolia/platform/json.hpp"
#include "magnolia/resources/material.hpp"
#include "magnolia/resources/model.hpp"
#include "magnolia/resources/resource.hpp"
#include "meshoptimizer.h"

// Include after model definitions
#include "magnolia/platform/serializer.hpp"

namespace mag
{
    constexpr str Material_File_Extension = ".mat.json";
    constexpr str Model_File_Extension = ".model.json";
    constexpr str Binary_File_Extension = ".model.bin";

    namespace tools
    {
        static b8 create_native_file(const str& output_directory, const ModelResource& model, str& imported_model_path);

        static b8 initialize_mesh(u32 mesh_idx, const aiMesh* ai_mesh, ModelResource& model);

        static void initialize_materials(const aiScene* ai_scene, const str& file_path, const str& output_directory,
                                         ModelResource& model);

        static void optimize_mesh(const std::vector<Vertex>& vertices, const std::vector<u32>& indices,
                                  ModelResource& model);

        static str find_texture(const str& material_name, const aiMaterial* ai_material, aiTextureType ai_type,
                                const str& directory);

        b8 import_model(const str& file_path, str& imported_model_path)
        {
            Assimp::Importer importer;

            const u32 flags = aiProcessPreset_TargetRealtime_Fast | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes |
                              aiProcess_PreTransformVertices | aiProcess_OptimizeMeshes | aiProcess_Debone |
                              aiProcess_RemoveRedundantMaterials;

            const aiScene* scene = importer.ReadFile(file_path, flags);
            if ((scene == nullptr) || (scene->mRootNode == nullptr) ||
                ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U))
            {
                LOG_ERROR("Failed to import model '{0}': {1}", file_path, importer.GetErrorString());
                return false;
            }

            if (!scene->HasMeshes())
            {
                LOG_ERROR("Model has no meshes '{0}'", file_path);
                return false;
            }

            ModelResource model = {};
            model.file_path = imported_model_path;
            model.name = scene->mRootNode->mName.C_Str();
            model.meshes.resize(scene->mNumMeshes);

            const std::span meshes_span(scene->mMeshes, scene->mNumMeshes);

            for (u32 i = 0; i < meshes_span.size(); i++)
            {
                const aiMesh* mesh = meshes_span[i];
                if (!initialize_mesh(i, mesh, model))
                {
                    return false;
                }
            }

            // Sort meshes by ascending order of material index
            std::ranges::sort(model.meshes.begin(), model.meshes.end(),
                              [](const Mesh& a, const Mesh& b) { return a.material_index < b.material_index; });

            const str output_directory = fs::path(file_path).parent_path() / fs::path("native");
            if (!fs::create_directories(output_directory))
            {
                LOG_ERROR("Failed to create directory: '{0}'", output_directory);
                return false;
            }

            initialize_materials(scene, file_path, output_directory, model);
            return create_native_file(output_directory, model, imported_model_path);
        }

        b8 create_native_file(const str& output_directory, const ModelResource& model, str& imported_model_path)
        {
            const str native_model_file_path = output_directory + "/" + model.name + Model_File_Extension;
            const str binary_file_path = output_directory + "/" + model.name + Binary_File_Extension;

            fs::json data;
            data["Type"] = "Model";
            data["Name"] = model.name;
            data["File"] = binary_file_path;
            data["NumVertices"] = model.vertices.size();
            data["NumIndices"] = model.indices.size();
            data["NumMeshes"] = model.meshes.size();

            for (const ref<MaterialResource>& material_ref : model.materials)
            {
                data["Materials"].push_back(material_ref->file_path);
            }

            // Write the data to the native file format
            if (!fs::write_json_data(native_model_file_path, data))
            {
                LOG_ERROR("Failed to create native model file: '{0}'", native_model_file_path);
                return false;
            }

            Buffer buffer;
            fs::serialize(buffer, model);

            // Write binary model data to file
            if (!fs::write_binary_data(binary_file_path, buffer))
            {
                LOG_ERROR("Failed to create binary model file: '{0}'", binary_file_path);
                return false;
            }

            imported_model_path = native_model_file_path;
            return true;
        }

        b8 initialize_mesh(const u32 mesh_idx, const aiMesh* ai_mesh, ModelResource& model)
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

            std::vector<u32> indices(ai_mesh->mNumFaces * 3ULL);

            const u32 index_count = indices.size();

            const std::span faces(ai_mesh->mFaces, ai_mesh->mNumFaces);

            // Indices
            for (u32 i = 0; i < faces.size(); i++)
            {
                const auto& face = faces[i];
                if (face.mNumIndices != 3)
                {
                    LOG_ERROR("Face is not a triangle");
                    return false;
                }

                const std::span indices_span(face.mIndices, face.mNumIndices);

                indices[(i * 3) + 0] = indices_span[0];
                indices[(i * 3) + 1] = indices_span[1];
                indices[(i * 3) + 2] = indices_span[2];
            }

            std::vector<Vertex> vertices(index_count);

            const std::span vertices_span(ai_mesh->mVertices, index_count);
            const std::span normals_span(ai_mesh->mNormals, index_count);
            const std::span tex_coords_span(ai_mesh->mTextureCoords[0], index_count);
            const std::span tangents_span(ai_mesh->mTangents, index_count);
            const std::span bitangents_span(ai_mesh->mBitangents, index_count);

            // Vertices - load with duplicates. The optimization step will create a better vertex/index buffer.
            for (u32 i = 0; i < index_count; i++)
            {
                Vertex vertex = {};

                const u32 idx = indices[i];
                vertex.position = {vertices_span[idx].x, vertices_span[idx].y, vertices_span[idx].z};
                vertex.normal = {normals_span[idx].x, normals_span[idx].y, normals_span[idx].z};
                vertex.tex_coords = {tex_coords_span[idx].x, tex_coords_span[idx].y};
                vertex.tangent = {tangents_span[idx].x, tangents_span[idx].y, tangents_span[idx].z};
                vertex.bitangent = {bitangents_span[idx].x, bitangents_span[idx].y, bitangents_span[idx].z};

                vertices[i] = vertex;
            }

            // Optimize
            optimize_mesh(vertices, indices, model);
            return true;
        }

        void optimize_mesh(const std::vector<Vertex>& vertices, const std::vector<u32>& indices, ModelResource& model)
        {
            const u32 vertex_count = vertices.size();
            const u32 index_count = indices.size();
            const f32 overdraw_threshold = 1.05F;

            std::vector<u32> remap(index_count);
            const u64 optimized_vertex_count = meshopt_generateVertexRemap(
                remap.data(), nullptr, index_count, vertices.data(), vertex_count, sizeof(Vertex));

            std::vector<Vertex> optimized_vertices(optimized_vertex_count);
            std::vector<u32> optimized_indices(index_count);

            // Remove duplicates
            meshopt_remapIndexBuffer(optimized_indices.data(), nullptr, index_count, remap.data());
            meshopt_remapVertexBuffer(optimized_vertices.data(), vertices.data(), vertex_count, sizeof(Vertex),
                                      remap.data());

            // Improve vertex locality
            meshopt_optimizeVertexCache(optimized_indices.data(), optimized_indices.data(), index_count,
                                        optimized_vertex_count);

            // Reduce pixel overdraw
            meshopt_optimizeOverdraw(optimized_indices.data(), optimized_indices.data(), index_count,
                                     &(optimized_vertices[0].position.x), optimized_vertex_count, sizeof(Vertex),
                                     overdraw_threshold);

            // Optimize vertex buffer access
            meshopt_optimizeVertexFetch(optimized_vertices.data(), optimized_indices.data(), index_count,
                                        optimized_vertices.data(), optimized_vertex_count, sizeof(Vertex));

            // Insert result into array
            model.vertices.insert(model.vertices.end(), optimized_vertices.begin(), optimized_vertices.end());
            model.indices.insert(model.indices.end(), optimized_indices.begin(), optimized_indices.end());
        }

        void initialize_materials(const aiScene* ai_scene, const str& file_path, const str& output_directory,
                                  ModelResource& model)
        {
            const str model_directory = fs::path(file_path).parent_path();

            model.materials.resize(ai_scene->mNumMaterials);

            const std::span materials_span(ai_scene->mMaterials, ai_scene->mNumMaterials);

            const u32 material_count = materials_span.size();
            for (u32 i = 0; i < material_count; i++)
            {
                const aiMaterial* ai_material = materials_span[i];
                str material_name = ai_material->GetName().C_Str();

                // Invalid material name, use placeholder instead
                if (material_name.empty())
                {
                    material_name = log::get_formatted_str("__Material_{0}_{1}__", i, material_count - 1);
                }

                const str material_file_path =
                    str(output_directory).append("/").append(material_name).append(Material_File_Extension);

                model.materials[i] = create_ref<MaterialResource>();
                model.materials[i]->name = material_name;
                model.materials[i]->file_path = material_file_path;

                // Write material data to file
                fs::json data;
                data["Type"] = "Material";
                data["Name"] = material_name;

                data["Textures"]["Albedo"] =
                    find_texture(material_name, ai_material, aiTextureType_DIFFUSE, model_directory);

                data["Textures"]["Normal"] =
                    find_texture(material_name, ai_material, aiTextureType_NORMALS, model_directory);

                data["Textures"]["Roughness"] =
                    find_texture(material_name, ai_material, aiTextureType_DIFFUSE_ROUGHNESS, model_directory);

                data["Textures"]["Metalness"] =
                    find_texture(material_name, ai_material, aiTextureType_METALNESS, model_directory);

                if (!fs::write_json_data(material_file_path, data))
                {
                    LOG_ERROR("Failed to create material file: {0}", material_file_path);
                    continue;
                }
            }
        }

        str find_texture(const str& material_name, const aiMaterial* ai_material, aiTextureType ai_type,
                         const str& directory)
        {
            // For some reason, assimp may identify normal textures as height textures
            u32 texture_count = ai_material->GetTextureCount(ai_type);
            if (ai_type == aiTextureType_NORMALS && texture_count == 0)
            {
                ai_type = aiTextureType_HEIGHT;
                texture_count = ai_material->GetTextureCount(ai_type);
            }

            str texture_name;

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

                LOG_INFO("Material '{0}': Found texture: '{1}'", material_name, texture_name);
                return texture_name;
            }

            LOG_WARNING("Material '{0}' has no texture of type '{1}'", material_name, std::to_string(ai_type));

            return texture_name;
        }

        b8 is_extension_supported(const str& extension_with_dot)
        {
            const Assimp::Importer importer;
            return importer.IsExtensionSupported(extension_with_dot);
        }
    };  // namespace tools
};  // namespace mag
