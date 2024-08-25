#include "resources/material_loader.hpp"

#include <fstream>

#include "core/logger.hpp"
#include "nlohmann/json.hpp"

namespace mag
{
    using json = nlohmann::ordered_json;

    std::shared_ptr<MaterialResource> MaterialLoader::load(const str& name)
    {
        auto it = material_resources.find(name);
        if (it != material_resources.end())
        {
            return it->second;
        }

        // Parse instructions from the json file
        std::ifstream file(name);

        if (!file.is_open())
        {
            LOG_ERROR("Failed to open material file: '{0}'", name);
            return nullptr;
        }

        // Parse the file
        const json data = json::parse(file);

        if (!data.contains("Material"))
        {
            LOG_ERROR("Material file '{0}' has no name", name);
            return nullptr;
        }

        if (!data.contains("Textures"))
        {
            LOG_ERROR("Material file '{0}' has no textures", name);
            return nullptr;
        }

        const str material_name = data["Material"];

        const json textures = data["Textures"];

        if (!textures.contains("Albedo") || !textures.contains("Normal"))
        {
            LOG_ERROR("Material file '{0}' has missing textures", name);
            return nullptr;
        }

        MaterialResource material_resource = {};
        material_resource.textures[TextureSlot::Albedo] = textures["Albedo"];
        material_resource.textures[TextureSlot::Normal] = textures["Normal"];

        LOG_SUCCESS("Loaded material: {0}", name);
        material_resources[name] = std::make_shared<MaterialResource>(material_resource);
        return material_resources[name];
    }

    void MaterialLoader::unload(const str& name)
    {
        auto it = material_resources.find(name);
        if (it != material_resources.end())
        {
            material_resources.erase(it);
            return;
        }

        LOG_WARNING("Material unload called with invalid name '{0}'", name);
    }
};  // namespace mag
