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
        auto& job_system = app.get_job_system();
        auto& renderer = app.get_renderer();
        auto& image_loader = app.get_image_loader();

        // Create a new texture
        Image* image = new Image();

        textures[name] = std::shared_ptr<Image>(image);

        // Try to create placeholder texture with the texture dimensions (otherwise use default settings)
        if (image_loader.get_info(name, &image->width, &image->height, reinterpret_cast<u32*>(&image->channels),
                                  &image->mip_levels))
        {
            image->pixels.resize(image->width * image->height * image->channels, image->pixels[0]);
        }

        // Send image data to the GPU
        auto renderer_image = renderer.add_image(image);

        // Load in another thread
        auto execute = [&image_loader, name, image]
        {
            // If the load fails we still have valid data
            image_loader.load(name, image);
        };

        // Callback when finished loading
        auto load_finished_callback = [image, renderer_image]
        {
            // Update the renderer image data
            renderer_image->set_pixels(image->pixels);
        };

        Job* load_job = new Job(execute, load_finished_callback);
        job_system.add_job(load_job);

        return textures[name];
    }

    std::shared_ptr<Image> TextureManager::get_default() { return get(DEFAULT_TEXTURE_NAME); }
};  // namespace mag
