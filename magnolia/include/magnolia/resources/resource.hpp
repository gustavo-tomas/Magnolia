#pragma once

#include <functional>
#include <map>
#include <memory>
#include <typeindex>

#include "magnolia/core/assert.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    enum class LoadingStatus
    {
        Pending,
        InProgress,
        Finished,
        Error
    };

    // Resource base
    struct IResource
    {
            virtual ~IResource() = default;
            LoadingStatus loading_status = LoadingStatus::Pending;
            str file_path = "";
            str name = "";
    };

    typedef std::function<void(const IResource*)> ResourceLoadedCallbackFn;

    struct TextureResource;
    struct MaterialResource;
    struct ModelResource;
    struct FontResource;
    struct AudioResource;
    struct ShaderResource;

    namespace resource
    {
        // Initialize all resource subsystems
        b8 initialize();

        // Shutdown all resource subsystems
        void shutdown();

        MAG_API ref<TextureResource> get_texture(const str& file_path);
        MAG_API ref<MaterialResource> get_material(const str& file_path);
        MAG_API ref<ModelResource> get_model(const str& file_path);
        MAG_API ref<FontResource> get_font(const str& file_path);
        MAG_API ref<AudioResource> get_audio(const str& file_path);
        MAG_API ref<ShaderResource> get_shader(const str& file_path);

        // Interface for a resource loader
        class IResourceLoader
        {
            public:
                virtual ~IResourceLoader() = default;
                virtual IResource* load(const str& file_path) = 0;
        };

        class ResourceManager
        {
            public:
                ResourceManager();
                ~ResourceManager();

                // Synchronous loading. Returns nullptr on error.
                template <typename T>
                ref<T> get_sync(const str& name)
                {
                    static_assert(std::is_base_of_v<IResource, T>, "T must derive from IResource");

                    const str resource_type_name = std::type_index(typeid(T)).name();
                    MAG_ASSERT(loaders.contains(std::type_index(typeid(T))),
                               "Loader for type '" + resource_type_name + "' is not registered");

                    // Check if resource is already loaded
                    auto it = resources.find(name);
                    if (it != resources.end())
                    {
                        return std::dynamic_pointer_cast<T>(it->second);
                    }

                    // Load resource
                    T* resource = load_resource<T>(name);

                    // @TODO: we are assuming that an invalid resource can be replaced by a 'new' one. This may not
                    // always be the case. A more robust approach might be necessary.
                    if (resource == nullptr)
                    {
                        resource = new T();
                    }

                    if (resource == nullptr)
                    {
                        resource->loading_status = LoadingStatus::Error;
                        LOG_ERROR("Failed to load resource: '{0}'", name);
                        return nullptr;
                    }

                    resource->loading_status = LoadingStatus::Finished;
                    resources[name] = ref<IResource>(resource);

                    return std::dynamic_pointer_cast<T>(resources[name]);
                }

                // Register a loader for a specific resource type
                template <typename T>
                void register_loader(unique<IResourceLoader> loader)
                {
                    static_assert(std::is_base_of_v<IResource, T>, "T must derive from IResource");
                    loaders[std::type_index(typeid(T))] = std::move(loader);
                }

            private:
                // Shorthand to load a resource
                template <typename T>
                T* load_resource(const str& file_path) const
                {
                    T* resource = reinterpret_cast<T*>(loaders.at(std::type_index(typeid(T)))->load(file_path));
                    return resource;
                }

                std::unordered_map<str, ref<IResource>> resources;
                std::unordered_map<std::type_index, unique<IResourceLoader>> loaders;
        };
    };  // namespace resource
};      // namespace mag
