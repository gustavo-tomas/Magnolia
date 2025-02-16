// this header on top
#include "resources/resource_loader.hpp"
// this header on top

#include "core/buffer.hpp"
#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "resources/model.hpp"

namespace mag
{
    namespace resource
    {
        b8 load(const str& file_path, Model* model)
        {
            // Reset model data
            *model = {};

            json data;

            if (!fs::read_json_data(file_path, data))
            {
                LOG_ERROR("Failed to load native model file: '{0}'", file_path);
                return false;
            }

            if (!data.contains("Name") || !data.contains("File") || !data.contains("Materials"))
            {
                LOG_ERROR("Model file '{0}' has incomplete fields", file_path);
                return false;
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
                return false;
            }

            // Extract juicy model data
            model->name = model_name;
            model->file_path = file_path;
            model->materials = materials;

            c8* model_data = buffer.cast<c8>();

            // Read vertices
            if (num_vertices > 0)
            {
                model->vertices.resize(num_vertices);
                memcpy(model->vertices.data(), model_data, VEC_SIZE_BYTES(model->vertices));
                model_data += VEC_SIZE_BYTES(model->vertices);
            }

            // Read indices
            if (num_indices > 0)
            {
                model->indices.resize(num_indices);
                memcpy(model->indices.data(), model_data, VEC_SIZE_BYTES(model->indices));
                model_data += VEC_SIZE_BYTES(model->indices);
            }

            // Read meshes
            if (num_meshes > 0)
            {
                model->meshes.resize(num_meshes);
                memcpy(model->meshes.data(), model_data, VEC_SIZE_BYTES(model->meshes));
                model_data += VEC_SIZE_BYTES(model->meshes);
            }

            LOG_SUCCESS("Loaded model: {0}", file_path);
            return true;
        }
    };  // namespace resource
};      // namespace mag
