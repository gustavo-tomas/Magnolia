#include "renderer/material.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"

namespace mag
{
    MaterialManager::MaterialManager()
    {
        auto& app = get_application();
        auto& texture_loader = app.get_texture_loader();

        // Create a default material
        Material* default_material = new Material();
        default_material->diffuse_texture = texture_loader.load("magnolia/assets/images/DefaultAlbedoSeamless.png");
        default_material->name = "Default";

        load(default_material);
    }

    std::shared_ptr<Material> MaterialManager::load(Material* material)
    {
        auto it = materials.find(material->name);

        if (it != materials.end())
        {
            LOG_WARNING("Material '{0}' already loaded", material->name);
            delete material;

            return it->second;
        }

        else
        {
            const auto mat = std::shared_ptr<Material>(material);
            materials[material->name] = mat;
            return mat;
        }
    }

    // @TODO: return a default material if name not found?
    std::shared_ptr<Material> MaterialManager::get(const str& name)
    {
        auto it = materials.find(name);

        if (it == materials.end())
        {
            LOG_ERROR("Material '{0}' not found, using default", name);
            return materials["Default"];
        }

        return it->second;
    }
};  // namespace mag
