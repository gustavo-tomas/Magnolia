#include "resources/model.hpp"

#include "core/application.hpp"
#include "renderer/renderer.hpp"
#include "renderer/test_model.hpp"
#include "resources/model_loader.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    ModelManager::ModelManager()
    {
        auto& app = get_application();
        auto& renderer = app.get_renderer();

        models[DEFAULT_MODEL_NAME] = create_ref<Model>();
        models[DEFAULT_MODEL_NAME]->name = "Default";
        models[DEFAULT_MODEL_NAME]->meshes = Cube().get_model().meshes;
        models[DEFAULT_MODEL_NAME]->vertices = Cube().get_model().vertices;
        models[DEFAULT_MODEL_NAME]->indices = Cube().get_model().indices;
        models[DEFAULT_MODEL_NAME]->materials = Cube().get_model().materials;

        // Send model data to the GPU
        renderer.upload_model(models[DEFAULT_MODEL_NAME].get());
    }

    ref<Model> ModelManager::get(const str& name)
    {
        auto it = models.find(name);
        if (it != models.end())
        {
            return it->second;
        }

        auto& app = get_application();
        auto& job_system = app.get_job_system();
        auto& model_loader = app.get_model_loader();
        auto& renderer = app.get_renderer();

        // Create a new model
        Model* model = new Model(*models[DEFAULT_MODEL_NAME]);
        models[name] = ref<Model>(model);

        // Send model data to the GPU
        renderer.upload_model(model);

        // Temporary model to load data into
        Model* transfer_model = new Model(*model);

        // Load in another thread
        auto execute = [&model_loader, name, transfer_model]
        {
            // If the load fails we still have valid data
            return model_loader.load(name, transfer_model);
        };

        // Callback when finished loading
        auto load_finished_callback = [&renderer, model, transfer_model](const b8 result)
        {
            // Update the model and renderer model data
            if (result == true)
            {
                *model = *transfer_model;
                renderer.update_model(model);
            }

            // We can dispose of the temporary model now
            delete transfer_model;
        };

        Job load_job = Job(execute, load_finished_callback);
        job_system.add_job(load_job);

        return models[name];
    }

    ref<Model> ModelManager::get_default() { return models[DEFAULT_MODEL_NAME]; }
};  // namespace mag
