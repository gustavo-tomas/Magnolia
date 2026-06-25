#include "magnolia/resources/texture.hpp"

#include "magnolia/core/buffer.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"
#include "stb/stb_image.h"

namespace mag
{
    namespace resource
    {
        b8 load_sync(const str& file_path, ResourceManager* rm, TextureResource* resource)
        {
            (void)rm;

            Buffer buffer;
            fs::read_binary_data(file_path, buffer);

            i32 tex_width = 0;
            i32 tex_height = 0;
            i32 tex_channels = 0;
            stbi_uc* pixels = stbi_load_from_memory(buffer.data.data(), static_cast<i32>(buffer.get_size()), &tex_width,
                                                    &tex_height, &tex_channels, STBI_rgb_alpha);

            if (pixels == nullptr)
            {
                LOG_ERROR("Failed to load image file: {0}", file_path);
                stbi_image_free(pixels);

                return false;
            }

            // @TODO: hardcoded channels
            tex_channels = 4;

            const u64 image_size = static_cast<u64>(tex_width * tex_height) * tex_channels;

            // Update image data
            resource->name = file_path;
            resource->file_path = file_path;
            resource->width = tex_width;
            resource->height = tex_height;
            resource->channels = tex_channels;
            resource->mip_levels = static_cast<u32>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;
            resource->pixels = std::vector<u8>(pixels, pixels + image_size);

            stbi_image_free(pixels);

            return true;
        }

        b8 get_image_info(const str& raw_file_path, u32* width, u32* height, u32* channels, u32* mip_levels)
        {
            i32 w = 0;
            i32 h = 0;
            i32 c = 0;

            const str file_path = fs::get_fixed_path(raw_file_path);
            const b8 result = stbi_info(file_path.c_str(), &w, &h, &c) != 0;

            *width = static_cast<u32>(w);
            *height = static_cast<u32>(h);
            *channels = static_cast<u32>(c);
            *mip_levels = static_cast<u32>(std::floor(std::log2(std::max(*width, *height)))) + 1;

            return result;
        }

        constexpr b8 is_image_extension_supported(const str& extension_with_dot)
        {
            return (extension_with_dot == ".jpeg" || extension_with_dot == ".png" || extension_with_dot == ".tga" ||
                    extension_with_dot == ".bmp" || extension_with_dot == ".psd" || extension_with_dot == ".gif" ||
                    extension_with_dot == ".hdr" || extension_with_dot == ".pic" || extension_with_dot == ".pnm");
        }
    };  // namespace resource
};  // namespace mag
