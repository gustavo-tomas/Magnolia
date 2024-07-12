#pragma once

#include <memory>

#include "renderer/image.hpp"

namespace mag
{
#define DEFAULT_MATERIAL_NAME "_MagDefaultMaterial"

    struct Material
    {
            enum TextureSlot
            {
                Albedo = 0,
                Normal,

                TextureCount
            };

            std::shared_ptr<Image> textures[TextureSlot::TextureCount];
            str name = "";

            // @TODO: create one per frame in flight
            // Each texture has a corresponding descriptor set for each frame in flight
            vk::DescriptorSet descriptor_sets[TextureSlot::TextureCount];
            vk::DescriptorSetLayout descriptor_set_layouts[TextureSlot::TextureCount];
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
