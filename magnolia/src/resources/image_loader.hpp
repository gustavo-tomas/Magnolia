#pragma once

#include "core/types.hpp"

namespace mag
{
    struct Image;

    class ImageLoader
    {
        public:
            b8 load(const str& file_path, Image* image);
            b8 get_info(const str& file_path, u32* width, u32* height, u32* channels, u32* mip_levels) const;
            b8 is_extension_supported(const str& extension_with_dot);
    };
};  // namespace mag
