// this header on top
#include "resources/resource_loader.hpp"
// this header on top

#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "resources/material.hpp"

namespace mag
{
    namespace resource
    {
        b8 load(const str& file_path, Material* material)
        {
            json data;

            if (!fs::read_json_data(file_path, data))
            {
                LOG_ERROR("Failed to load material: '{0}'", file_path);
                return false;
            }

            if (!data.contains("Name"))
            {
                LOG_ERROR("Material file '{0}' has no name", file_path);
                return false;
            }

            if (!data.contains("Textures"))
            {
                LOG_ERROR("Material file '{0}' has no textures", file_path);
                return false;
            }

            const str material_name = data["Name"];

            const json textures = data["Textures"];

            if (!textures.contains("Albedo") || !textures.contains("Normal"))
            {
                LOG_ERROR("Material file '{0}' has missing textures", file_path);
                return false;
            }

            // Set material data
            material->name = file_path;
            material->textures[TextureSlot::Albedo] = textures["Albedo"];
            material->textures[TextureSlot::Normal] = textures["Normal"];
            material->textures[TextureSlot::Roughness] = textures["Roughness"];
            material->textures[TextureSlot::Metalness] = textures["Metalness"];

            LOG_SUCCESS("Loaded material: {0}", file_path);
            return true;
        }
    };  // namespace resource
};      // namespace mag
