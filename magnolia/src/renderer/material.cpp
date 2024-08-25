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
        auto& texture_manager = app.get_texture_manager();

        // Try loading the material
        auto material_resource = material_loader.load(name);
        if (material_resource == nullptr)
        {
            LOG_ERROR("Material '{0}' not found, using default", name);

            material_resource = material_loader.load(DEFAULT_MATERIAL_NAME);
            ASSERT(material_resource, "Default material has not been loaded");
        }

        Material* material = new Material();
        material->name = name;
        material->textures = material_resource->textures;

        // @TODO: temporary? idk if descriptors should be created here
        {
            std::vector<std::shared_ptr<Image>> textures;
            for (const auto& texture_name : material->textures)
            {
                textures.push_back(texture_manager.get(texture_name.second));
            }

            // @TODO: hardcoded binding (0)
            DescriptorBuilder::create_descriptor_for_textures(0, textures, material->descriptor_set,
                                                              material->descriptor_set_layout);
        }
        // @TODO: temporary? idk if descriptors should be created here

        materials[name] = std::shared_ptr<Material>(material);
        return materials[name];
    }

    std::shared_ptr<Material> MaterialManager::get_default() { return get(DEFAULT_MATERIAL_NAME); }
};  // namespace mag
