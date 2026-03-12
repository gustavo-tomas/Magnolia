#include "magnolia/resources/model.hpp"

#include "magnolia/core/buffer.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"
#include "magnolia/platform/json.hpp"
#include "magnolia/platform/serializer.hpp"
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

namespace mag::fs
{
    void to_binary(std::ostringstream& ss, const Vertex& data)
    {
        to_binary(ss, data.position);
        to_binary(ss, data.normal);
        to_binary(ss, data.tex_coords);
        to_binary(ss, data.tangent);
        to_binary(ss, data.bitangent);
    }

    void from_binary(std::istringstream& ss, Vertex& data)
    {
        from_binary(ss, data.position);
        from_binary(ss, data.normal);
        from_binary(ss, data.tex_coords);
        from_binary(ss, data.tangent);
        from_binary(ss, data.bitangent);
    }

    void to_binary(std::ostringstream& ss, const Mesh& data)
    {
        to_binary(ss, data.base_vertex);
        to_binary(ss, data.base_index);
        to_binary(ss, data.index_count);
        to_binary(ss, data.material_index);
        to_binary(ss, data.aabb_min);
        to_binary(ss, data.aabb_max);
    }

    void from_binary(std::istringstream& ss, Mesh& data)
    {
        from_binary(ss, data.base_vertex);
        from_binary(ss, data.base_index);
        from_binary(ss, data.index_count);
        from_binary(ss, data.material_index);
        from_binary(ss, data.aabb_min);
        from_binary(ss, data.aabb_max);
    }

    void to_binary(std::ostringstream& ss, const ModelResource& data)
    {
        to_binary(ss, data.vertices);
        to_binary(ss, data.indices);
        to_binary(ss, data.meshes);
    }

    void from_binary(std::istringstream& ss, ModelResource& data)
    {
        from_binary(ss, data.vertices);
        from_binary(ss, data.indices);
        from_binary(ss, data.meshes);
    }
};  // namespace mag::fs
