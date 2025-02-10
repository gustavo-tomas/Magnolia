#include "resources/image.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "renderer/renderer.hpp"
#include "resources/resource_loader.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    TextureManager::TextureManager()
    {
        auto& app = get_application();
        auto& renderer = app.get_renderer();

        textures[DEFAULT_ALBEDO_TEXTURE_NAME] = create_ref<Image>();
        textures[DEFAULT_NORMAL_TEXTURE_NAME] = create_ref<Image>();
        textures[DEFAULT_ROUGHNESS_TEXTURE_NAME] = create_ref<Image>();
        textures[DEFAULT_METALNESS_TEXTURE_NAME] = create_ref<Image>();

        for (u64 i = 0; i < textures[DEFAULT_ALBEDO_TEXTURE_NAME]->pixels.size(); i += 4)
        {
            auto& pixels_normal = textures[DEFAULT_NORMAL_TEXTURE_NAME]->pixels;

            pixels_normal[i + 0] = 128;
            pixels_normal[i + 1] = 128;
            pixels_normal[i + 2] = 255;
            pixels_normal[i + 3] = 255;

            auto& pixels_roughness = textures[DEFAULT_ROUGHNESS_TEXTURE_NAME]->pixels;

            pixels_roughness[i + 0] = 128;
            pixels_roughness[i + 1] = 128;
            pixels_roughness[i + 2] = 128;
            pixels_roughness[i + 3] = 128;

            auto& pixels_metalness = textures[DEFAULT_METALNESS_TEXTURE_NAME]->pixels;

            pixels_metalness[i + 0] = 0;
            pixels_metalness[i + 1] = 0;
            pixels_metalness[i + 2] = 0;
            pixels_metalness[i + 3] = 0;
        }

        renderer.upload_image(textures[DEFAULT_ALBEDO_TEXTURE_NAME].get());
        renderer.upload_image(textures[DEFAULT_NORMAL_TEXTURE_NAME].get());
        renderer.upload_image(textures[DEFAULT_ROUGHNESS_TEXTURE_NAME].get());
        renderer.upload_image(textures[DEFAULT_METALNESS_TEXTURE_NAME].get());
    }

    ref<Image> TextureManager::get(const str& name)
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

        // Create a new texture
        Image* image = new Image();

        textures[name] = ref<Image>(image);

        // Try to create placeholder texture with the texture dimensions (otherwise use default settings)
        if (resource::get_image_info(name, &image->width, &image->height, reinterpret_cast<u32*>(&image->channels),
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
        auto execute = [name, transfer_image]
        {
            // If the load fails we still have valid data
            return resource::load(name, transfer_image);
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

    ref<Image> TextureManager::get_default() { return textures[DEFAULT_ALBEDO_TEXTURE_NAME]; }
};  // namespace mag
