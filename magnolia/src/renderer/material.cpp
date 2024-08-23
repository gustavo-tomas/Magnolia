#include "renderer/material.hpp"

#include "core/logger.hpp"

namespace mag
{
    MaterialManager::MaterialManager()
    {
        // Create a default material
        Material* default_material = new Material();

        default_material->name = DEFAULT_MATERIAL_NAME;
        default_material->textures[Material::TextureSlot::Albedo] = "magnolia/assets/images/DefaultAlbedoSeamless.png";
        default_material->textures[Material::TextureSlot::Normal] = "magnolia/assets/images/DefaultNormal.png";

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

    b8 MaterialManager::exists(const str& name) const { return materials.contains(name); }

    std::shared_ptr<Material> MaterialManager::get(const str& name)
    {
        auto it = materials.find(name);

        if (it == materials.end())
        {
            LOG_ERROR("Material '{0}' not found, using default", name);
            return materials[DEFAULT_MATERIAL_NAME];
        }

        return it->second;
    }
};  // namespace mag
