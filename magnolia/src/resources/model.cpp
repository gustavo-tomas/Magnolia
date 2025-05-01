#include "resources/model.hpp"

#include <map>

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
                ResourceLoadedCallbackFn on_model_loaded;
        };

        static State* state = nullptr;

        b8 initialize_model_subsystem()
        {
            state = new State();
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
            Model* model = new Model();
            model->loading_status = LoadingStatus::InProgress;
            state->models[name] = ref<Model>(model);

            // Temporary model to load data into
            Model* transfer_model = new Model(*model);

            // Load in another thread
            auto execute = [name, transfer_model]
            {
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
                // Update the model data
                if (result == true)
                {
                    *model = *transfer_model;
                    state->on_model_loaded(model);
                }

                // We can dispose of the temporary model now
                delete transfer_model;
            };

            Job load_job = Job(execute, load_finished_callback);
            thread::add_job(load_job);

            return state->models[name];
        }

        void set_on_model_loaded_callback(const ResourceLoadedCallbackFn& callback)
        {
            state->on_model_loaded = callback;
        }
    };  // namespace resource
};      // namespace mag
