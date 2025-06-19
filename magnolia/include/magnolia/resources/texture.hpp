#pragma once

#include <vector>

#include "magnolia/core/types.hpp"
#include "magnolia/resources/resource.hpp"

namespace mag
{
#define DEFAULT_ALBEDO_TEXTURE_NAME "__mag_default_albedo_texture__"
#define DEFAULT_NORMAL_TEXTURE_NAME "__mag_default_normal_texture__"
#define DEFAULT_ROUGHNESS_TEXTURE_NAME "__mag_default_roughness_texture__"
#define DEFAULT_METALNESS_TEXTURE_NAME "__mag_default_metalness_texture__"

    struct TextureResource : public IResource
    {
            u8 channels = 4;
            u32 width = 64;
            u32 height = 64;
            u32 mip_levels = 1;
            std::vector<u8> pixels = std::vector<u8>(64 * 64 * 4, 153);
    };

    namespace resource
    {
        class TextureLoader : public IResourceLoader
        {
            public:
                TextureLoader();
                ~TextureLoader();

                virtual IResource* load(const str& file_path) override;
        };

        b8 get_image_info(const str& file_path, u32* width, u32* height, u32* channels, u32* mip_levels);
        b8 is_image_extension_supported(const str& extension_with_dot);
    };  // namespace resource
};      // namespace mag
