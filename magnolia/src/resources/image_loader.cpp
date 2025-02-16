// this header on top
#include "resources/resource_loader.hpp"
// this header on top

#include <set>

#include "core/buffer.hpp"
#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "resources/image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace mag
{
    namespace resource
    {
        b8 load(const str& file_path, Image* image)
        {
            if (!image)
            {
                LOG_ERROR("Invalid image ptr");
                return false;
            }

            Buffer buffer;
            fs::read_binary_data(file_path, buffer);

            i32 tex_width = 0, tex_height = 0, tex_channels = 0;
            stbi_uc* pixels = stbi_load_from_memory(buffer.data.data(), buffer.get_size(), &tex_width, &tex_height,
                                                    &tex_channels, STBI_rgb_alpha);

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

        b8 get_image_info(const str& raw_file_path, u32* width, u32* height, u32* channels, u32* mip_levels)
        {
            const str file_path = fs::get_fixed_path(raw_file_path);

            const b8 result = stbi_info(file_path.c_str(), reinterpret_cast<i32*>(width),
                                        reinterpret_cast<i32*>(height), reinterpret_cast<i32*>(channels));

            // @TODO: hardcoded channels
            *channels = 4;

            *mip_levels = static_cast<u32>(std::floor(std::log2(std::max(*width, *height)))) + 1;

            return result;
        }

        b8 is_image_extension_supported(const str& extension_with_dot)
        {
            // Extensions supported by stb
            static const std::set<str> supported_formats = {".jpeg", ".png", ".tga", ".bmp", ".psd",
                                                            ".gif",  ".hdr", ".pic", ".pnm"};

            return supported_formats.contains(extension_with_dot);
        }
    };  // namespace resource
};      // namespace mag
