#include "resources/image.hpp"

#include "core/application.hpp"

namespace mag
{
#define DEFAULT_TEXTURE_NAME "magnolia/assets/images/DefaultAlbedoSeamless.png"

    std::shared_ptr<Image> TextureManager::get(const str& name)
    {
        // Texture found
        auto it = textures.find(name);
        if (it != textures.end())
        {
            return it->second;
        }

        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& image_loader = app.get_image_loader();

        // Create a new texture
        u32 width, height, channels, mip_levels, color = 200;

        Image* image = new Image();

        // Try to create placeholder texture with the texture dimensions
        if (image_loader.get_info(name, &width, &height, &channels, &mip_levels))
        {
            image->width = width;
            image->height = height;
            image->channels = channels;
            image->mip_levels = mip_levels;
            image->pixels = std::vector<u8>(width * height * channels, color);
        }

        // Use default dimensions on failure
        else
        {
            image->width = 64;
            image->height = 64;
            image->channels = 4;
            image->mip_levels = 1;
            image->pixels = std::vector<u8>(64 * 64 * 4, color);
        }

        // If the load fails we still have valid data
        image_loader.load(name, image);

        // Send image data to the GPU
        renderer.add_image(image);

        textures[name] = std::shared_ptr<Image>(image);
        return textures[name];
    }

    std::shared_ptr<Image> TextureManager::get_default() { return get(DEFAULT_TEXTURE_NAME); }
};  // namespace mag
