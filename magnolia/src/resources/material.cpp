#include "resources/material.hpp"

#include "resources/resource.hpp"
#include "resources/resource_loader.hpp"
#include "resources/texture.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    namespace resource
    {
        struct State
        {
                std::map<str, ref<MaterialResource>> materials;
                ResourceLoadedCallbackFn on_material_loaded;
        };

        static State* state = nullptr;

        b8 initialize_material_subsystem()
        {
            state = new State();

            state->materials[DEFAULT_MATERIAL_NAME] = create_ref<MaterialResource>();
            state->materials[DEFAULT_MATERIAL_NAME]->name = "Default";
            state->materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Albedo] = DEFAULT_ALBEDO_TEXTURE_NAME;
            state->materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Normal] = DEFAULT_NORMAL_TEXTURE_NAME;
            state->materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Roughness] = DEFAULT_ROUGHNESS_TEXTURE_NAME;
            state->materials[DEFAULT_MATERIAL_NAME]->textures[TextureSlot::Metalness] = DEFAULT_METALNESS_TEXTURE_NAME;

            return state != nullptr;
        }

        void shutdown_material_subsystem()
        {
            state->materials.clear();
            delete state;
        }

        ref<MaterialResource> get_material(const str& name)
        {
            auto it = state->materials.find(name);
            if (it != state->materials.end())
            {
                return it->second;
            }

            // Create a new material
            MaterialResource* material = new MaterialResource(*state->materials[DEFAULT_MATERIAL_NAME]);
            material->loading_status = LoadingStatus::InProgress;
            state->materials[name] = ref<MaterialResource>(material);

            // Temporary material to load data into
            MaterialResource* transfer_material = new MaterialResource(*material);

            // Load in another thread
            auto execute = [name, transfer_material]
            {
                // If the load fails we still have valid data
                const b8 result = resource::load(name, transfer_material);

                if (result)
                {
                    transfer_material->loading_status = LoadingStatus::Finished;
                }

                else
                {
                    transfer_material->loading_status = LoadingStatus::Error;
                }

                return result;
            };

            // Callback when finished loading
            auto load_finished_callback = [material, transfer_material](const b8 result)
            {
                // Update the material and the renderer material data
                if (result == true)
                {
                    *material = *transfer_material;
                    state->on_material_loaded(material);
                }

                // We can dispose of the temporary material now
                delete transfer_material;
            };

            Job load_job = Job(execute, load_finished_callback);
            thread::add_job(load_job);

            return state->materials[name];
        }

        ref<MaterialResource> get_default_material() { return state->materials[DEFAULT_MATERIAL_NAME]; }

        void set_on_material_loaded_callback(const ResourceLoadedCallbackFn& callback)
        {
            state->on_material_loaded = callback;
        }
    };  // namespace resource
};      // namespace mag
