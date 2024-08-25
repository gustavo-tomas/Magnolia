#pragma once

#include <map>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"

namespace mag
{
#define DEFAULT_MATERIAL_NAME "magnolia/assets/materials/default_material.mat.json"

    enum class TextureSlot
    {
        Albedo = 0,
        Normal,

        TextureCount
    };

    struct Material
    {
            std::map<TextureSlot, str> textures;
            str name = "";

            // @TODO: create one per frame in flight if materials should change between frames
            vk::DescriptorSet descriptor_set;
            vk::DescriptorSetLayout descriptor_set_layout;
    };

    class MaterialManager
    {
        public:
            std::shared_ptr<Material> get(const str& name);
            std::shared_ptr<Material> get_default();

        private:
            std::map<str, std::shared_ptr<Material>> materials;
    };
};  // namespace mag
