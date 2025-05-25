#include "resources/image.hpp"

#include <map>

#include "core/logger.hpp"
#include "resources/resource.hpp"
#include "resources/resource_loader.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    namespace resource
    {
        struct State
        {
                std::map<str, ref<TextureResource>> textures;
                ResourceLoadedCallbackFn on_texture_loaded;
        };

        static State* state = nullptr;

        b8 initialize_texture_subsystem()
        {
            state = new State();
            return state != nullptr;
        }

        void shutdown_texture_subsystem()
        {
            state->textures.clear();
            delete state;
        }

        ref<TextureResource> get_texture(const str& name)
        {
            // Texture found
            auto it = state->textures.find(name);
            if (it != state->textures.end())
            {
                return it->second;
            }

            // Create a new texture
            TextureResource* image = new TextureResource();
            image->loading_status = LoadingStatus::InProgress;
            state->textures[name] = ref<TextureResource>(image);

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

            // Temporary image to load data into
            TextureResource* transfer_image = new TextureResource(*image);

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

        void set_on_texture_loaded_callback(const ResourceLoadedCallbackFn& callback)
        {
            state->on_texture_loaded = callback;
        }
    };  // namespace resource
};      // namespace mag
