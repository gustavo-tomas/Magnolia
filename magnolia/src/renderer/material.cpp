#include "renderer/material.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"

namespace mag
{
    MaterialManager::MaterialManager()
    {
        auto& app = get_application();
        auto& texture_loader = app.get_texture_manager();

        // Create a default material
        Material* default_material = new Material();

        default_material->name = DEFAULT_MATERIAL_NAME;

        default_material->textures[Material::TextureSlot::Albedo] =
            texture_loader.load("magnolia/assets/images/DefaultAlbedoSeamless.png", TextureType::Albedo);

        default_material->textures[Material::TextureSlot::Normal] =
            texture_loader.load("magnolia/assets/images/DefaultNormal.png", TextureType::Normal);

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
