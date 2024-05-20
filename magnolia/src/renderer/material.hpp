#pragma once

#include <memory>

#include "renderer/image.hpp"

namespace mag
{
    struct Material
    {
            std::shared_ptr<Image> diffuse_texture = {};
            str name = "";  // @TODO: meh
    };

    class MaterialLoader
    {
        public:
            // std::shared_ptr<Material> load(const str& file); // @TODO: make a material file one day
            std::shared_ptr<Material> load(Material* material);

            // @TODO: return a default material if name not found?
            std::shared_ptr<Material> get(const str& name);

        private:
            std::map<str, std::shared_ptr<Material>> materials;
    };
};  // namespace mag
