#pragma once

#include <memory>

#include "core/logger.hpp"
#include "renderer/image.hpp"

namespace mag
{
    struct Material
    {
            std::shared_ptr<Image> diffuse_texture = {};
            vec3 color = vec3(1.0f);
    };

    class MaterialLoader
    {
        public:
            // std::shared_ptr<Material> load(const str& file); // @TODO: make a material file one day
            std::shared_ptr<Material> load(const str& name, Material* material)
            {
                auto it = materials.find(name);

                if (it != materials.end())
                {
                    LOG_WARNING("Material is already loaded");
                    delete material;

                    return it->second;
                }

                else
                {
                    const auto mat = std::shared_ptr<Material>(material);
                    materials[name] = mat;
                    return mat;
                }
            };

            // @TODO: return a default material if name not found?
            std::shared_ptr<Material> get(const str& name)
            {
                auto it = materials.find(name);

                if (it != materials.end())
                {
                    return it->second;
                }

                return nullptr;
            };

        private:
            std::map<str, std::shared_ptr<Material>> materials;
    };
};  // namespace mag
