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

    class TextureManager
    {
        public:
            std::shared_ptr<Image> get(const str& name);
            std::shared_ptr<Image> get_default();

        private:
            std::map<str, std::shared_ptr<Image>> textures;
    };
};  // namespace mag
