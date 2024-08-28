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

        // Else load image from disk and create a new texture
        Image* image = image_loader.load(name);

        if (image == nullptr)
        {
            LOG_ERROR("Texture '{0}' not found, using default", name);

            image = image_loader.load(DEFAULT_TEXTURE_NAME);
            ASSERT(image, "Default texture has not been loaded");
        }

        // Send image data to the GPU
        renderer.add_image(image);

        textures[name] = std::shared_ptr<Image>(image);
        return textures[name];
    }

    std::shared_ptr<Image> TextureManager::get_default() { return get(DEFAULT_TEXTURE_NAME); }
};  // namespace mag
