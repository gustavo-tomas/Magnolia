#pragma once

#include <memory>

#include "renderer/image.hpp"

namespace mag
{
#define DEFAULT_MATERIAL_NAME "MagDefaultMaterial"

    struct Material
    {
            std::vector<std::shared_ptr<Image>> textures;
            str name = "";  // @TODO: meh

            // Helpers
            static const u32 DEFAULT_ALBEDO_TEXTURE = 0;
            static const u32 DEFAULT_NORMAL_TEXTURE = 1;
    };

    class MaterialManager
    {
        public:
            MaterialManager();
            ~MaterialManager() = default;

            // std::shared_ptr<Material> load(const str& file); // @TODO: make a material file one day
            std::shared_ptr<Material> load(Material* material);
            b8 exists(const str& name) const;

            std::shared_ptr<Material> get(const str& name);

        private:
            std::map<str, std::shared_ptr<Material>> materials;
    };
};  // namespace mag
