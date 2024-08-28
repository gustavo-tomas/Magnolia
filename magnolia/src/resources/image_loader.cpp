#include "resources/image_loader.hpp"

#include "core/logger.hpp"
#include "resources/image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace mag
{
    Image* ImageLoader::load(const str& file_path)
    {
        i32 tex_width = 0, tex_height = 0, tex_channels = 0;

        stbi_uc* pixels = stbi_load(file_path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
        if (pixels == NULL)
        {
            LOG_ERROR("Failed to load image file: {0}", file_path);
            stbi_image_free(pixels);

            return nullptr;
        }

        // @TODO: hardcoded channels
        tex_channels = 4;

        const u64 image_size = tex_width * tex_height * tex_channels;
        const u32 mip_levels = static_cast<u32>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

        Image* image = new Image();
        image->channels = tex_channels;
        image->width = tex_width;
        image->height = tex_height;
        image->mip_levels = mip_levels;
        image->pixels = std::vector<u8>(pixels, pixels + image_size);

        stbi_image_free(pixels);

        return image;
    }
};  // namespace mag
