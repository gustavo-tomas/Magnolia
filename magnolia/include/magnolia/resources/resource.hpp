#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>

#include "magnolia/core/assert.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    enum class LoadingStatus : u8
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
            str file_path;
            str name;
    };

    using ResourceLoadedCallbackFn = std::function<void(const IResource*)>;

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

        MAG_API ref<TextureResource> get_texture(const str& file_path, const b8 reload = false);
        MAG_API ref<MaterialResource> get_material(const str& file_path, const b8 reload = false);
        MAG_API ref<ModelResource> get_model(const str& file_path, const b8 reload = false);
        MAG_API ref<FontResource> get_font(const str& file_path, const b8 reload = false);
        MAG_API ref<AudioResource> get_audio(const str& file_path, const b8 reload = false);
        MAG_API ref<ShaderResource> get_shader(const str& file_path, const b8 reload = false);

        // Interface for a resource loader
        class IResourceLoader
        {
            public:
                virtual ~IResourceLoader() = default;
                virtual IResource* load(const str& file_path) = 0;
        };

        // @TODO: it might be a good idea to create our own types with mutex variants to make life easier. They are
        // mostly wrappers for the STL anyway.

        template <typename Key, typename Value>
        class Map
        {
            public:
                using iterator = typename std::unordered_map<Key, Value>::iterator;

                Map() = default;

                ~Map()
                {
                    std::unique_lock<std::mutex> lock(map_mutex);
                    map.clear();
                }

                b8 contains(const Key& key)
                {
                    std::unique_lock<std::mutex> lock(map_mutex);
                    return map.contains(key);
                }

                iterator find(const Key& key)
                {
                    std::unique_lock<std::mutex> lock(map_mutex);
                    return map.find(key);
                }

                iterator end()
                {
                    std::unique_lock<std::mutex> lock(map_mutex);
                    return map.end();
                }

                Value& operator[](const Key& key)
                {
                    std::unique_lock<std::mutex> lock(map_mutex);
                    return map[key];
                }

            private:
                std::unordered_map<Key, Value> map;
                std::mutex map_mutex;
        };

        class ResourceManager
        {
            public:
                ResourceManager();
                ~ResourceManager();

                // Synchronous loading. Returns nullptr on error.
                template <typename T>
                ref<T> get_sync(const str& name, const b8 reload = false)
                {
                    static_assert(std::is_base_of_v<IResource, T>, "T must derive from IResource");

                    const str resource_type_name = std::type_index(typeid(T)).name();
                    MAG_ASSERT(loaders.contains(std::type_index(typeid(T))), "Loader for type '{}' is not registered",
                               resource_type_name);

                    // Check if resource is already loaded
                    auto it = resources.find(name);
                    if (!reload && it != resources.end())
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
                        LOG_ERROR("Failed to load resource: '{0}'", name);
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
                T* load_resource(const str& file_path)
                {
                    T* resource = reinterpret_cast<T*>(loaders[std::type_index(typeid(T))]->load(file_path));
                    return resource;
                }

                Map<str, ref<IResource>> resources;
                Map<std::type_index, unique<IResourceLoader>> loaders;
        };
    };  // namespace resource
};  // namespace mag
