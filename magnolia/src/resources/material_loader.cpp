#include "resources/material_loader.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "resources/material.hpp"

namespace mag
{
    b8 MaterialLoader::load(const str& name, Material* material)
    {
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        json data;

        if (!file_system.read_json_data(name, data))
        {
            LOG_ERROR("Failed to load material: '{0}'", name);
            return false;
        }

        if (!data.contains("Name"))
        {
            LOG_ERROR("Material file '{0}' has no name", name);
            return false;
        }

        if (!data.contains("Textures"))
        {
            LOG_ERROR("Material file '{0}' has no textures", name);
            return false;
        }

        const str material_name = data["Name"];

        const json textures = data["Textures"];

        if (!textures.contains("Albedo") || !textures.contains("Normal"))
        {
            LOG_ERROR("Material file '{0}' has missing textures", name);
            return false;
        }

        // Set material data
        material->name = name;
        material->textures[TextureSlot::Albedo] = textures["Albedo"];
        material->textures[TextureSlot::Normal] = textures["Normal"];

        LOG_SUCCESS("Loaded material: {0}", name);
        return true;
    }
};  // namespace mag
