#include "resources/model.hpp"

#include <map>

#include "renderer/renderer.hpp"
#include "renderer/test_model.hpp"
#include "resources/resource.hpp"
#include "resources/resource_loader.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    namespace resource
    {
        struct State
        {
                std::map<str, ref<Model>> models;
        };

        static State* state = nullptr;

        b8 initialize_model_subsystem()
        {
            state = new State();

            state->models[DEFAULT_MODEL_NAME] = create_ref<Model>();
            state->models[DEFAULT_MODEL_NAME]->name = "Default";
            state->models[DEFAULT_MODEL_NAME]->meshes = Cube().get_model().meshes;
            state->models[DEFAULT_MODEL_NAME]->vertices = Cube().get_model().vertices;
            state->models[DEFAULT_MODEL_NAME]->indices = Cube().get_model().indices;
            state->models[DEFAULT_MODEL_NAME]->materials = Cube().get_model().materials;

            // Send model data to the GPU
            gfx::upload_model(state->models[DEFAULT_MODEL_NAME].get());
            state->models[DEFAULT_MODEL_NAME]->loading_status = LoadingStatus::UploadedToGpu;

            return state != nullptr;
        }

        void shutdown_model_subsystem()
        {
            state->models.clear();
            delete state;
        }

        ref<Model> get_model(const str& name)
        {
            auto it = state->models.find(name);
            if (it != state->models.end())
            {
                return it->second;
            }

            // Create a new model
            Model* model = new Model(*state->models[DEFAULT_MODEL_NAME]);
            model->loading_status = LoadingStatus::InProgress;
            state->models[name] = ref<Model>(model);

            // Send model data to the GPU
            gfx::upload_model(model);

            // Temporary model to load data into
            Model* transfer_model = new Model(*model);

            // Load in another thread
            auto execute = [name, transfer_model]
            {
                // If the load fails we still have valid data
                const b8 result = resource::load(name, transfer_model);

                if (result)
                {
                    transfer_model->loading_status = LoadingStatus::Finished;
                }

                else
                {
                    transfer_model->loading_status = LoadingStatus::Error;
                }

                return result;
            };

            // Callback when finished loading
            auto load_finished_callback = [model, transfer_model](const b8 result)
            {
                // Update the model and renderer model data
                if (result == true)
                {
                    *model = *transfer_model;
                    gfx::update_model(model);
                    model->loading_status = LoadingStatus::UploadedToGpu;
                }

                // We can dispose of the temporary model now
                delete transfer_model;
            };

            Job load_job = Job(execute, load_finished_callback);
            thread::add_job(load_job);

            return state->models[name];
        }

        ref<Model> get_default_model() { return state->models[DEFAULT_MODEL_NAME]; }
    };  // namespace resource
};      // namespace mag
