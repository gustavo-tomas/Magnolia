#include "renderer/material.hpp"

#include "core/logger.hpp"

namespace mag
{
    // std::shared_ptr<Material> load(const str& file); // @TODO: make a material file one day
    std::shared_ptr<Material> MaterialLoader::load(Material* material)
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
    };

    // @TODO: return a default material if name not found?
    std::shared_ptr<Material> MaterialLoader::get(const str& name)
    {
        auto it = materials.find(name);
        ASSERT(it != materials.end(), "Material '" + name + "' not found");

        return it->second;
    };
};  // namespace mag
