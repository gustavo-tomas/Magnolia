#include "magnolia/resources/model.hpp"

#include "magnolia/core/buffer.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"
#include "magnolia/platform/json.hpp"
#include "magnolia/resources/material.hpp"
#include "magnolia/resources/resource.hpp"

namespace mag
{
    namespace resource
    {
        ModelLoader::ModelLoader(ResourceManager* resource_manager) : resource_manager(resource_manager) {}

        ModelLoader::~ModelLoader() = default;

        IResource* ModelLoader::load_sync(const str& file_path)
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

            fs::deserialize(buffer, *model);

            // Get dependencies
            for (const str& material : materials)
            {
                const ref<MaterialResource>& material_ref = resource_manager->get_sync<MaterialResource>(material);
                model->materials.push_back(material_ref);
            }

            LOG_SUCCESS("Loaded model: {0}", file_path);
            return model;
        }
    };  // namespace resource
};  // namespace mag
