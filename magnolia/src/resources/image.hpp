#pragma once

#include <map>
#include <vector>

#include "core/types.hpp"

namespace mag
{
#define DEFAULT_ALBEDO_TEXTURE_NAME "__mag_default_albedo_texture__"
#define DEFAULT_NORMAL_TEXTURE_NAME "__mag_default_normal_texture__"
#define DEFAULT_ROUGHNESS_TEXTURE_NAME "__mag_default_roughness_texture__"
#define DEFAULT_METALNESS_TEXTURE_NAME "__mag_default_metalness_texture__"

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
            TextureManager();

            ref<Image> get(const str& name);
            ref<Image> get_default();

        private:
            std::map<str, ref<Image>> textures;
    };
};  // namespace mag
