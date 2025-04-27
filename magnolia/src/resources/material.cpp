#include "resources/material.hpp"

#include "resources/image.hpp"
#include "resources/resource_loader.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    namespace resource
    {
        struct State
        {
                std::map<str, ref<Material>> materials;
        };

        static State* state = nullptr;

        b8 initialize_material_subsystem()
        {
            state = new State();

            state->materials[DEFAULT_MATERIAL_NAME] = create_ref<Material>();
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

        ref<Material> get_material(const str& name)
        {
            auto it = state->materials.find(name);
            if (it != state->materials.end())
            {
                return it->second;
            }

            // Create a new material
            Material* material = new Material(*state->materials[DEFAULT_MATERIAL_NAME]);
            state->materials[name] = ref<Material>(material);

            // Temporary material to load data into
            Material* transfer_material = new Material(*material);

            // Load in another thread
            auto execute = [name, transfer_material]
            {
                // If the load fails we still have valid data
                transfer_material->loading_state = MaterialLoadingState::LoadingInProgress;
                return resource::load(name, transfer_material);
            };

            // Callback when finished loading
            auto load_finished_callback = [material, transfer_material](const b8 result)
            {
                // Update the material and the renderer material data
                if (result == true)
                {
                    transfer_material->loading_state = MaterialLoadingState::LoadingFinished;
                    *material = *transfer_material;
                }

                // We can dispose of the temporary material now
                delete transfer_material;
            };

            Job load_job = Job(execute, load_finished_callback);
            thread::add_job(load_job);

            return state->materials[name];
        }

        ref<Material> get_default_material() { return state->materials[DEFAULT_MATERIAL_NAME]; }
    };  // namespace resource
};      // namespace mag
