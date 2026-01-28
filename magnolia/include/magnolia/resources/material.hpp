#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/resources/resource.hpp"

namespace mag
{
#define DEFAULT_MATERIAL_NAME "__mag_default_material__"

    enum class TextureSlot : u8
    {
        Albedo = 0,
        Normal,
        Roughness,
        Metalness,

        TextureCount
    };

    struct MaterialResource : public IResource
    {
            std::unordered_map<TextureSlot, ref<TextureResource>> textures;
    };

    namespace resource
    {
        class MaterialLoader : public IResourceLoader
        {
            public:
                MaterialLoader(ResourceManager& resource_manager);
                ~MaterialLoader() override;

                IResource* load(const str& file_path) override;

            private:
                ResourceManager& resource_manager;
        };

        ref<MaterialResource> MAG_API get_default_material();
    };  // namespace resource
};  // namespace mag
