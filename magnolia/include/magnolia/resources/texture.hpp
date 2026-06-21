#pragma once

#include <vector>

#include "magnolia/core/types.hpp"
#include "magnolia/resources/resource.hpp"

namespace mag
{
    struct TextureResource : public IResource
    {
            u8 channels = 4;
            u32 width = 64;
            u32 height = 64;
            u32 mip_levels = 1;
            std::vector<u8> pixels = std::vector<u8>(64L * 64 * 4, 153);
    };

    namespace resource
    {
        b8 load_sync(const str& file_path, ResourceManager* rm, TextureResource* resource);
        b8 get_image_info(const str& file_path, u32* width, u32* height, u32* channels, u32* mip_levels);
        b8 is_image_extension_supported(const str& extension_with_dot);
    };  // namespace resource
};  // namespace mag
