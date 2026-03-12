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
        b8 load_sync(const str& file_path, ResourceManager* rm, MaterialResource* resource);
    };  // namespace resource
};  // namespace mag
