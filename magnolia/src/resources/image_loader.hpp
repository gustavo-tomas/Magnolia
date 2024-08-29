#pragma once

#include "core/types.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    struct Image;

    class ImageLoader
    {
        public:
            void load(const str& file_path, Image* image, const JobCallbackFn& callback);
            b8 get_info(const str& file_path, u32* width, u32* height, u32* channels, u32* mip_levels) const;
    };
};  // namespace mag
