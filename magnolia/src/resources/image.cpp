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

        else
        {
            LOG_ERROR("Failed to retrieve image dimensions for '{0}'", name);
        }

        // Send image data to the GPU
        renderer.upload_image(image);

        // Temporary image to load data into
        Image* transfer_image = new Image(*image);

        // Load in another thread
        auto execute = [&image_loader, name, transfer_image]
        {
            // If the load fails we still have valid data
            return image_loader.load(name, transfer_image);
        };

        // Callback when finished loading
        auto load_finished_callback = [image, transfer_image, &renderer](const b8 result)
        {
            // Update the image and the renderer image data
            if (result == true)
            {
                *image = *transfer_image;
                renderer.update_image(image);
            }

            // We can dispose of the temporary image now
            delete transfer_image;
        };

        Job load_job = Job(execute, load_finished_callback);
        job_system.add_job(load_job);

        return textures[name];
    }

    std::shared_ptr<Image> TextureManager::get_default() { return get(DEFAULT_TEXTURE_NAME); }
};  // namespace mag
