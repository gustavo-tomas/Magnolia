#include "resources/resource.hpp"

#include "resources/audio.hpp"
#include "resources/font.hpp"
#include "resources/material.hpp"
#include "resources/model.hpp"
#include "resources/shader.hpp"
#include "resources/texture.hpp"

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

        ref<TextureResource> get_texture(const str& file_path)
        {
            return state->rm.get_sync<TextureResource>(file_path);
        }

        ref<MaterialResource> get_material(const str& file_path)
        {
            return state->rm.get_sync<MaterialResource>(file_path);
        }

        ref<ModelResource> get_model(const str& file_path) { return state->rm.get_sync<ModelResource>(file_path); }

        ref<FontResource> get_font(const str& file_path) { return state->rm.get_sync<FontResource>(file_path); }

        ref<AudioResource> get_audio(const str& file_path) { return state->rm.get_sync<AudioResource>(file_path); }

        ref<ShaderResource> get_shader(const str& file_path) { return state->rm.get_sync<ShaderResource>(file_path); }

        ResourceManager::ResourceManager()
        {
            // Register loaders
            register_loader<TextureResource>(create_unique<TextureLoader>());
            register_loader<MaterialResource>(create_unique<MaterialLoader>(*this));
            register_loader<ModelResource>(create_unique<ModelLoader>(*this));
            register_loader<FontResource>(create_unique<FontLoader>());
            register_loader<ShaderResource>(create_unique<ShaderLoader>());
            register_loader<AudioResource>(create_unique<AudioLoader>());
        }

        ResourceManager::~ResourceManager() = default;
    };  // namespace resource
};      // namespace mag
