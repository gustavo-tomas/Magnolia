#pragma once

#include "core/types.hpp"

namespace mag
{
    struct Image;

    class ImageLoader
    {
        public:
            void load(const str& file_path, Image* image);
            b8 get_info(const str& file_path, u32* width, u32* height, u32* channels, u32* mip_levels) const;
    };
};  // namespace mag
