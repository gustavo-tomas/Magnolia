#include "resources/material_loader.hpp"

#include <fstream>

#include "core/logger.hpp"
#include "nlohmann/json.hpp"
#include "resources/material.hpp"

namespace mag
{
    using json = nlohmann::ordered_json;

    b8 MaterialLoader::load(const str& name, Material* material)
    {
        // Parse instructions from the json file
        std::ifstream file(name);

        if (!file.is_open())
        {
            LOG_ERROR("Failed to open material file: '{0}'", name);
            return false;
        }

        // Parse the file
        const json data = json::parse(file);

        if (!data.contains("Material"))
        {
            LOG_ERROR("Material file '{0}' has no name", name);
            return false;
        }

        if (!data.contains("Textures"))
        {
            LOG_ERROR("Material file '{0}' has no textures", name);
            return false;
        }

        const str material_name = data["Material"];

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
