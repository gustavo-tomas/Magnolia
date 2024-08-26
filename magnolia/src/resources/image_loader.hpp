#pragma once

#include "core/types.hpp"

namespace mag
{
    struct Image;

    class ImageLoader
    {
        public:
            Image* load(const str& file_path);
    };
};  // namespace mag
