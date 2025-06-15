#include "resources/material.hpp"

#include "platform/file_system.hpp"
#include "resources/resource.hpp"
#include "resources/texture.hpp"

namespace mag
{
    namespace resource
    {
        MaterialLoader::MaterialLoader()
        {
            // materials[DEFAULT_MATERIAL_NAME] = create_ref<MaterialResource>();
            // materials[DEFAULT_MATERIAL_NAME]->name = "Default";
            // materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Albedo] = DEFAULT_ALBEDO_TEXTURE_NAME;
            // materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Normal] = DEFAULT_NORMAL_TEXTURE_NAME;
            // materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Roughness] = DEFAULT_ROUGHNESS_TEXTURE_NAME;
            // materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Metalness] = DEFAULT_METALNESS_TEXTURE_NAME;
        }

        MaterialLoader::~MaterialLoader() {}

        IResource* MaterialLoader::load(const str& file_path)
        {
            MaterialResource* material = new MaterialResource();

            fs::json data;

            if (!fs::read_json_data(file_path, data))
            {
                LOG_ERROR("Failed to load material: '{0}'", file_path);
                delete material;
                return nullptr;
            }

            if (!data.contains("Name"))
            {
                LOG_ERROR("Material file '{0}' has no name", file_path);
                delete material;
                return nullptr;
            }

            if (!data.contains("Textures"))
            {
                LOG_ERROR("Material file '{0}' has no textures", file_path);
                delete material;
                return nullptr;
            }

            const str material_name = data["Name"];

            const fs::json textures = data["Textures"];

            if (!textures.contains("Albedo") || !textures.contains("Normal"))
            {
                LOG_ERROR("Material file '{0}' has missing textures", file_path);
                delete material;
                return nullptr;
            }

            // Set material data
            material->name = material_name;
            material->file_path = file_path;
            material->textures[TextureSlot::Albedo] = textures["Albedo"];
            material->textures[TextureSlot::Normal] = textures["Normal"];
            material->textures[TextureSlot::Roughness] = textures["Roughness"];
            material->textures[TextureSlot::Metalness] = textures["Metalness"];

            // Set dependencies
            for (const auto& [slot, texture] : material->textures)
            {
                const ResourceDependency dep = ResourceDependency(std::type_index(typeid(TextureResource)), texture);
                material->dependencies.push_back(dep);
            }

            LOG_SUCCESS("Loaded material: {0}", file_path);
            return material;
        }
    };  // namespace resource
};      // namespace mag
