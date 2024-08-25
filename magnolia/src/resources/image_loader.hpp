#pragma once

#include <map>
#include <memory>
#include <vector>

#include "core/types.hpp"

namespace mag
{
    struct Image
    {
            u8 channels;
            u32 width;
            u32 height;
            u32 mip_levels;
            std::vector<u8> pixels;
    };

    class ImageLoader
    {
        public:
            std::shared_ptr<Image> load(const str& name);
            void unload(const str& name);

        private:
            std::map<str, std::shared_ptr<Image>> image_resources;
    };
};  // namespace mag
