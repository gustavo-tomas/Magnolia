#include "resources/model_loader.hpp"

#include "core/application.hpp"
#include "core/file_system.hpp"
#include "core/logger.hpp"
#include "resources/model.hpp"

namespace mag
{
    b8 ModelLoader::load(const str& file_path, Model* model)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        // Reset model data
        *model = {};

        json data;

        if (!file_system.read_json_data(file_path, data))
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

        c8* model_data = buffer.cast<c8>();

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

        LOG_SUCCESS("Loaded model: {0}", file_path);
        return true;
    }
};  // namespace mag
