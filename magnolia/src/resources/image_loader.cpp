#include "resources/image_loader.hpp"

#include "core/logger.hpp"
#include "resources/image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace mag
{
    b8 ImageLoader::load(const str& file_path, Image* image)
    {
        if (!image)
        {
            LOG_ERROR("Invalid image ptr");
            return false;
        }

        i32 tex_width = 0, tex_height = 0, tex_channels = 0;

        stbi_uc* pixels = stbi_load(file_path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
        if (pixels == NULL)
        {
            LOG_ERROR("Failed to load image file: {0}", file_path);
            stbi_image_free(pixels);

            return false;
        }

        // @TODO: hardcoded channels
        tex_channels = 4;

        const u64 image_size = tex_width * tex_height * tex_channels;

        // Update image data
        image->width = tex_width;
        image->height = tex_height;
        image->channels = tex_channels;
        image->mip_levels = static_cast<u32>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;
        image->pixels = std::vector<u8>(pixels, pixels + image_size);

        stbi_image_free(pixels);

        return true;
    }

    b8 ImageLoader::get_info(const str& file_path, u32* width, u32* height, u32* channels, u32* mip_levels) const
    {
        const b8 result = stbi_info(file_path.c_str(), reinterpret_cast<i32*>(width), reinterpret_cast<i32*>(height),
                                    reinterpret_cast<i32*>(channels));

        // @TODO: hardcoded channels
        *channels = 4;

        *mip_levels = static_cast<u32>(std::floor(std::log2(std::max(*width, *height)))) + 1;

        return result;
    }
};  // namespace mag
