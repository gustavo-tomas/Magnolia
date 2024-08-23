#include "resources/image_loader.hpp"

#include "core/logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace mag
{
    void ImageLoader::unload(const str& name)
    {
        auto it = image_resources.find(name);
        if (it != image_resources.end())
        {
            image_resources.erase(name);
            return;
        }

        LOG_WARNING("Image unload called with invalid name '{0}'", name);
    }

    std::shared_ptr<ImageResource> ImageLoader::load(const str& file)
    {
        auto it = image_resources.find(file);
        if (it != image_resources.end())
        {
            return it->second;
        }

        i32 tex_width = 0, tex_height = 0, tex_channels = 0;

        stbi_uc* pixels = stbi_load(file.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
        if (pixels == NULL)
        {
            LOG_ERROR("Failed to load image file: {0}", file);
            stbi_image_free(pixels);

            return nullptr;
        }

        // @TODO: hardcoded channels
        tex_channels = 4;

        const u64 image_size = tex_width * tex_height * tex_channels;
        const u32 mip_levels = static_cast<u32>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

        ImageResource image_resource = {};
        image_resource.channels = tex_channels;
        image_resource.width = tex_width;
        image_resource.height = tex_height;
        image_resource.mip_levels = mip_levels;
        image_resource.pixels = std::vector<u8>(pixels, pixels + image_size);

        stbi_image_free(pixels);

        image_resources[file] = std::make_shared<ImageResource>(image_resource);
        return image_resources[file];
    }
};  // namespace mag
