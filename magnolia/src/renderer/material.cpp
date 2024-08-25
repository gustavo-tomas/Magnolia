#include "renderer/material.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"

namespace mag
{
    std::shared_ptr<Material> MaterialManager::get(const str& name)
    {
        auto it = materials.find(name);
        if (it != materials.end())
        {
            return it->second;
        }

        auto& app = get_application();
        auto& material_loader = app.get_material_loader();
        auto& renderer = app.get_renderer();

        // Try loading the material
        Material* material = material_loader.load(name);

        if (material == nullptr)
        {
            LOG_ERROR("Material '{0}' not found, using default", name);

            material = material_loader.load(DEFAULT_MATERIAL_NAME);
            ASSERT(material, "Default material has not been loaded");
        }

        // Send material data to the GPU
        renderer.add_material(material);

        materials[name] = std::shared_ptr<Material>(material);
        return materials[name];
    }

    std::shared_ptr<Material> MaterialManager::get_default() { return get(DEFAULT_MATERIAL_NAME); }
};  // namespace mag
