#include "magnolia/resources/material.hpp"

#include "magnolia/platform/file_system.hpp"
#include "magnolia/resources/resource.hpp"
#include "magnolia/resources/texture.hpp"

namespace mag
{
    namespace resource
    {
        MaterialLoader::MaterialLoader(ResourceManager* resource_manager) : resource_manager(resource_manager)
        {
            // materials[DEFAULT_MATERIAL_NAME] = create_ref<MaterialResource>();
            // materials[DEFAULT_MATERIAL_NAME]->name = "Default";
            // materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Albedo] = DEFAULT_ALBEDO_TEXTURE_NAME;
            // materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Normal] = DEFAULT_NORMAL_TEXTURE_NAME;
            // materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Roughness] = DEFAULT_ROUGHNESS_TEXTURE_NAME;
            // materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Metalness] = DEFAULT_METALNESS_TEXTURE_NAME;
        }

        MaterialLoader::~MaterialLoader() = default;

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

            std::unordered_map<TextureSlot, str> textures_map;

            textures_map[TextureSlot::Albedo] = textures["Albedo"];
            textures_map[TextureSlot::Normal] = textures["Normal"];
            textures_map[TextureSlot::Roughness] = textures["Roughness"];
            textures_map[TextureSlot::Metalness] = textures["Metalness"];

            // Get dependencies
            for (const auto& [slot, texture] : textures_map)
            {
                material->textures[slot] = resource_manager->get_sync<TextureResource>(texture);
            }

            LOG_SUCCESS("Loaded material: {0}", file_path);
            return material;
        }
    };  // namespace resource
};  // namespace mag
