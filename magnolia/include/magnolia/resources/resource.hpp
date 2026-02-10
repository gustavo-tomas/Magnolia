#pragma once

#include <functional>
#include <memory>
#include <typeindex>

#include "magnolia/core/assert.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"
#include "magnolia/threads/containers.hpp"
#include "magnolia/threads/job_system.hpp"

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

    using ResourceLoadedCallbackFn = std::function<void(ref<IResource>)>;

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

        MAG_API void get_model_async(const str& file_path, const JobGroupHandle job_group,
                                     const ResourceLoadedCallbackFn& callback, const b8 reload = false);

        MAG_API void get_texture_async(const str& file_path, const JobGroupHandle job_group,
                                       const ResourceLoadedCallbackFn& callback, const b8 reload = false);

        // Interface for a resource loader
        class IResourceLoader
        {
            public:
                virtual ~IResourceLoader() = default;
                virtual IResource* load_sync(const str& file_path) = 0;
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

                // Asynchronous loading. Basically calls sync loading in another thread. Must be called from the main
                // thread.
                template <typename T>
                void get_async(const str& name, const JobGroupHandle job_group,
                               const ResourceLoadedCallbackFn& callback, const b8 reload = false)
                {
                    // Check if resource is loaded
                    auto it = resources.find(name);
                    if (!reload && it != resources.end())
                    {
                        callback(std::dynamic_pointer_cast<T>(it->second));
                        return;
                    }

                    // Check if resource is loading
                    if (loading_map.contains(name))
                    {
                        loading_map[name].push(callback);
                        return;
                    }

                    // First time loading this resource
                    loading_map[name].push(callback);

                    Job job(
                        [this, name, reload]()
                        {
                            JobData data = {};
                            data.data = get_sync<T>(name, reload);
                            data.result = data.data.has_value();

                            return data;
                        },

                        [this, name](const JobData data)
                        {
                            while (!loading_map[name].empty())
                            {
                                ResourceLoadedCallbackFn queued_callback = loading_map[name].pop();
                                queued_callback(std::any_cast<ref<T>>(data.data));
                            }

                            loading_map.erase(name);
                        });

                    thread::add_job(job_group, job);
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
                    T* resource = reinterpret_cast<T*>(loaders[std::type_index(typeid(T))]->load_sync(file_path));
                    return resource;
                }

                thread::Map<str, ref<IResource>> resources;
                thread::Map<std::type_index, unique<IResourceLoader>> loaders;
                thread::Map<str, thread::Queue<ResourceLoadedCallbackFn>> loading_map;
        };
    };  // namespace resource
};  // namespace mag
