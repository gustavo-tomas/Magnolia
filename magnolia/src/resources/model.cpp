#include "magnolia/resources/model.hpp"

#include "magnolia/core/buffer.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/core/memory.hpp"
#include "magnolia/platform/file_system.hpp"
#include "magnolia/resources/material.hpp"
#include "magnolia/resources/resource.hpp"

namespace mag
{
    namespace resource
    {
        ModelLoader::ModelLoader(ResourceManager& resource_manager) : resource_manager(resource_manager) {}

        ModelLoader::~ModelLoader() = default;

        IResource* ModelLoader::load(const str& file_path)
        {
            ModelResource* model = new ModelResource();

            fs::json data;

            if (!fs::read_json_data(file_path, data))
            {
                LOG_ERROR("Failed to load native model file: '{0}'", file_path);
                delete model;
                return nullptr;
            }

            if (!data.contains("Name") || !data.contains("File") || !data.contains("Materials"))
            {
                LOG_ERROR("Model file '{0}' has incomplete fields", file_path);
                delete model;
                return nullptr;
            }

            const str model_name = data["Name"];
            const str binary_file_path = data["File"];
            const std::vector<str> materials = data["Materials"];

            const u32 num_vertices = data["NumVertices"].get<u32>();
            const u32 num_indices = data["NumIndices"].get<u32>();
            const u32 num_meshes = data["NumMeshes"].get<u32>();

            Buffer buffer;
            const b8 result = fs::read_binary_data(binary_file_path, buffer);

            if (!result)
            {
                LOG_ERROR("Failed to load native model binary file: '{0}'", binary_file_path);
                delete model;
                return nullptr;
            }

            // Extract juicy model data
            model->name = model_name;
            model->file_path = file_path;

            c8* model_data = buffer.cast<c8>();

            // Read vertices
            if (num_vertices > 0)
            {
                model->vertices.resize(num_vertices);
                const u64 data_size = VEC_SIZE_BYTES(model->vertices);
                mem::copy(model->vertices.data(), data_size, model_data, data_size, data_size);
                model_data += data_size;
            }

            // Read indices
            if (num_indices > 0)
            {
                model->indices.resize(num_indices);
                const u64 data_size = VEC_SIZE_BYTES(model->indices);
                mem::copy(model->indices.data(), data_size, model_data, data_size, data_size);
                model_data += VEC_SIZE_BYTES(model->indices);
            }

            // Read meshes
            if (num_meshes > 0)
            {
                model->meshes.resize(num_meshes);
                const u64 data_size = VEC_SIZE_BYTES(model->meshes);
                mem::copy(model->meshes.data(), data_size, model_data, data_size, data_size);
                model_data += VEC_SIZE_BYTES(model->meshes);
            }

            // Get dependencies
            for (const str& material : materials)
            {
                const ref<MaterialResource>& material_ref = resource_manager.get_sync<MaterialResource>(material);
                model->materials.push_back(material_ref);
            }

            LOG_SUCCESS("Loaded model: {0}", file_path);
            return model;
        }
    };  // namespace resource
};      // namespace mag
