#include "resources/resource.hpp"

#include "core/assert.hpp"
#include "core/logger.hpp"
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
            register_loader<MaterialResource>(create_unique<MaterialLoader>());
            register_loader<ModelResource>(create_unique<ModelLoader>());
            register_loader<FontResource>(create_unique<FontLoader>());
            register_loader<ShaderResource>(create_unique<ShaderLoader>());
            register_loader<AudioResource>(create_unique<AudioLoader>());
        }

        ResourceManager::~ResourceManager() = default;

        void ResourceManager::load_dependencies(const ResourceDependency& dep)
        {
            if (resources.contains(dep.file_path))
            {
                return;
            }

            // @TODO: some would argue that this approach is a bit funny, weird even. I agree.

            IResource* resource = nullptr;

            if (dep.type == std::type_index(typeid(TextureResource)))
            {
                resource = load_resource<TextureResource>(dep.file_path);
            }

            else if (dep.type == std::type_index(typeid(MaterialResource)))
            {
                resource = load_resource<MaterialResource>(dep.file_path);
            }

            else if (dep.type == std::type_index(typeid(ModelResource)))
            {
                resource = load_resource<ModelResource>(dep.file_path);
            }

            else if (dep.type == std::type_index(typeid(FontResource)))
            {
                resource = load_resource<FontResource>(dep.file_path);
            }

            else if (dep.type == std::type_index(typeid(AudioResource)))
            {
                resource = load_resource<AudioResource>(dep.file_path);
            }

            else if (dep.type == std::type_index(typeid(ShaderResource)))
            {
                resource = load_resource<ShaderResource>(dep.file_path);
            }

            else
            {
                MAG_ASSERT(false, "Unhandled resource dependency type");
                return;
            }

            if (resource != nullptr)
            {
                MAG_ASSERT(resource->file_path == dep.file_path,
                           "Mismatch between resource and dependency file path: " + resource->file_path + " vs " +
                               dep.file_path);

                resource->loading_status = LoadingStatus::Finished;
                resources[dep.file_path] = ref<IResource>(resource);

                LOG_SUCCESS("Loaded resource dependency: '{0}'", dep.file_path);
            }

            else
            {
                LOG_ERROR("Failed to load resource dependency: '{0}'", dep.file_path);
                return;
            }

            for (const ResourceDependency& dep : resource->dependencies)
            {
                load_dependencies(dep);
            }
        }
    };  // namespace resource
};      // namespace mag
