#include "resources/image.hpp"

#include <map>

#include "core/logger.hpp"
#include "renderer/renderer.hpp"
#include "resources/resource.hpp"
#include "resources/resource_loader.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    namespace resource
    {
        struct State
        {
                std::map<str, ref<Image>> textures;
                ResourceLoadedCallbackFn on_texture_loaded;
        };

        static State* state = nullptr;

        b8 initialize_texture_subsystem()
        {
            state = new State();

            state->textures[DEFAULT_ALBEDO_TEXTURE_NAME] = create_ref<Image>();
            state->textures[DEFAULT_NORMAL_TEXTURE_NAME] = create_ref<Image>();
            state->textures[DEFAULT_ROUGHNESS_TEXTURE_NAME] = create_ref<Image>();
            state->textures[DEFAULT_METALNESS_TEXTURE_NAME] = create_ref<Image>();

            for (u64 i = 0; i < state->textures[DEFAULT_ALBEDO_TEXTURE_NAME]->pixels.size(); i += 4)
            {
                auto& pixels_normal = state->textures[DEFAULT_NORMAL_TEXTURE_NAME]->pixels;

                pixels_normal[i + 0] = 128;
                pixels_normal[i + 1] = 128;
                pixels_normal[i + 2] = 255;
                pixels_normal[i + 3] = 255;

                auto& pixels_roughness = state->textures[DEFAULT_ROUGHNESS_TEXTURE_NAME]->pixels;

                pixels_roughness[i + 0] = 128;
                pixels_roughness[i + 1] = 128;
                pixels_roughness[i + 2] = 128;
                pixels_roughness[i + 3] = 128;

                auto& pixels_metalness = state->textures[DEFAULT_METALNESS_TEXTURE_NAME]->pixels;

                pixels_metalness[i + 0] = 0;
                pixels_metalness[i + 1] = 0;
                pixels_metalness[i + 2] = 0;
                pixels_metalness[i + 3] = 0;
            }

            gfx::upload_image(state->textures[DEFAULT_ALBEDO_TEXTURE_NAME].get());
            gfx::upload_image(state->textures[DEFAULT_NORMAL_TEXTURE_NAME].get());
            gfx::upload_image(state->textures[DEFAULT_ROUGHNESS_TEXTURE_NAME].get());
            gfx::upload_image(state->textures[DEFAULT_METALNESS_TEXTURE_NAME].get());

            return state != nullptr;
        }

        void shutdown_texture_subsystem()
        {
            state->textures.clear();
            delete state;
        }

        ref<Image> get_texture(const str& name)
        {
            // Texture found
            auto it = state->textures.find(name);
            if (it != state->textures.end())
            {
                return it->second;
            }

            // Create a new texture
            Image* image = new Image();
            image->loading_status = LoadingStatus::InProgress;
            state->textures[name] = ref<Image>(image);

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
            gfx::upload_image(image);

            // Temporary image to load data into
            Image* transfer_image = new Image(*image);

            // Load in another thread
            auto execute = [name, transfer_image]
            {
                const b8 result = resource::load(name, transfer_image);

                if (result)
                {
                    transfer_image->loading_status = LoadingStatus::Finished;
                }

                else
                {
                    transfer_image->loading_status = LoadingStatus::Error;
                }

                return result;
            };

            // Callback when finished loading
            auto load_finished_callback = [image, transfer_image](const b8 result)
            {
                // Update the image data
                if (result == true)
                {
                    *image = *transfer_image;
                    state->on_texture_loaded(image);
                }

                // We can dispose of the temporary image now
                delete transfer_image;
            };

            Job load_job = Job(execute, load_finished_callback);
            thread::add_job(load_job);

            return state->textures[name];
        }

        ref<Image> get_default_texture() { return state->textures[DEFAULT_ALBEDO_TEXTURE_NAME]; }

        void set_on_texture_loaded_callback(const ResourceLoadedCallbackFn& callback)
        {
            state->on_texture_loaded = callback;
        }
    };  // namespace resource
};      // namespace mag
