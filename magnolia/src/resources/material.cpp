#include "magnolia/resources/material.hpp"

#include "magnolia/platform/file_system.hpp"
#include "magnolia/platform/json.hpp"
#include "magnolia/resources/texture.hpp"

namespace mag
{
    namespace resource
    {
        b8 load_sync(const str& file_path, ResourceManager* rm, MaterialResource* resource)
        {
            fs::json data;

            if (!fs::read_json_data(file_path, data))
            {
                LOG_ERROR("Failed to load material: '{0}'", file_path);
                return false;
            }

            if (!data.contains("Name") || !data.contains("Textures"))
            {
                LOG_ERROR("Material file '{0}' has incomplete fields", file_path);
                return false;
            }

            const str material_name = data["Name"];

            const fs::json textures = data["Textures"];

            if (!textures.contains("Albedo") || !textures.contains("Normal"))
            {
                LOG_ERROR("Material file '{0}' has missing textures", file_path);
                return false;
            }

            // Set material data
            resource->name = material_name;
            resource->file_path = file_path;

            std::unordered_map<TextureSlot, str> textures_map;

            textures_map[TextureSlot::Albedo] = textures["Albedo"];
            textures_map[TextureSlot::Normal] = textures["Normal"];
            textures_map[TextureSlot::Roughness] = textures["Roughness"];
            textures_map[TextureSlot::Metalness] = textures["Metalness"];

            // Get dependencies
            for (const auto& [slot, texture] : textures_map)
            {
                resource->textures[slot] = rm->get_sync<TextureResource>(texture);
            }

            LOG_SUCCESS("Loaded material: {0}", file_path);
            return true;
        }
    };  // namespace resource
};  // namespace mag
