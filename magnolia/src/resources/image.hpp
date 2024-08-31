#pragma once

#include <map>
#include <memory>
#include <vector>

#include "core/types.hpp"

namespace mag
{
    struct Image
    {
            u8 channels = 4;
            u32 width = 64;
            u32 height = 64;
            u32 mip_levels = 1;
            std::vector<u8> pixels = std::vector<u8>(64 * 64 * 4, 153);
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
