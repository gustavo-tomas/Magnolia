#include "magnolia/resources/resource.hpp"

#include "magnolia/resources/audio.hpp"
#include "magnolia/resources/font.hpp"
#include "magnolia/resources/material.hpp"
#include "magnolia/resources/model.hpp"
#include "magnolia/resources/shader.hpp"
#include "magnolia/resources/texture.hpp"

namespace mag
{
    namespace resource
    {
        struct ResourceSystemState
        {
                ResourceManager rm;
        };

        static ResourceSystemState* state = nullptr;

        b8 initialize()
        {
            state = new ResourceSystemState();
            return state != nullptr;
        }

        void shutdown() { delete state; }

        ref<TextureResource> get_texture(const str& file_path, const b8 reload)
        {
            return state->rm.get_sync<TextureResource>(file_path, reload);
        }

        ref<MaterialResource> get_material(const str& file_path, const b8 reload)
        {
            return state->rm.get_sync<MaterialResource>(file_path, reload);
        }

        ref<ModelResource> get_model(const str& file_path, const b8 reload)
        {
            return state->rm.get_sync<ModelResource>(file_path, reload);
        }

        ref<FontResource> get_font(const str& file_path, const b8 reload)
        {
            return state->rm.get_sync<FontResource>(file_path, reload);
        }

        ref<AudioResource> get_audio(const str& file_path, const b8 reload)
        {
            return state->rm.get_sync<AudioResource>(file_path, reload);
        }

        ref<ShaderResource> get_shader(const str& file_path, const b8 reload)
        {
            return state->rm.get_sync<ShaderResource>(file_path, reload);
        }

        void get_model_async(const str& file_path, const b8 reload, const ResourceLoadedCallbackFn& callback)
        {
            state->rm.get_async<ModelResource>(file_path, reload, callback);
        }

        void get_texture_async(const str& file_path, const b8 reload, const ResourceLoadedCallbackFn& callback)
        {
            state->rm.get_async<TextureResource>(file_path, reload, callback);
        }

        ResourceManager::ResourceManager()
        {
            // Register loaders
            register_loader<TextureResource>(create_unique<TextureLoader>());
            register_loader<MaterialResource>(create_unique<MaterialLoader>(this));
            register_loader<ModelResource>(create_unique<ModelLoader>(this));
            register_loader<FontResource>(create_unique<FontLoader>());
            register_loader<ShaderResource>(create_unique<ShaderLoader>());
            register_loader<AudioResource>(create_unique<AudioLoader>());
        }

        ResourceManager::~ResourceManager() = default;
    };  // namespace resource
};  // namespace mag
