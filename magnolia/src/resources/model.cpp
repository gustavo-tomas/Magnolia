#include "resources/model.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "renderer/test_model.hpp"

namespace mag
{
#define DEFAULT_MODEL_NAME "magnolia/assets/models/DefaultCube.model.json"

    std::shared_ptr<Model> ModelManager::get(const str& name)
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
        Model* model = new Model();

        models[name] = std::shared_ptr<Model>(model);

        // Create placeholder model with cube data
        Cube placeholder;
        const auto& placeholder_model = placeholder.get_model();

        *model = placeholder_model;
        model->file_path = "Placeholder";
        model->name = "Placeholder";

        // Send model data to the GPU
        renderer.add_model(model);

        // Temporary model to load data into
        Model* transfer_model = new Model();

        // Load in another thread
        auto execute = [&model_loader, name, transfer_model]
        {
            // If the load fails we still have valid data
            model_loader.load(name, transfer_model);
        };

        // Callback when finished loading
        auto load_finished_callback = [&renderer, model, transfer_model]
        {
            // Update the model and renderer model data
            *model = *transfer_model;
            renderer.update_model(model);

            // We can dispose of the temporary model now
            delete transfer_model;
        };

        Job* load_job = new Job(execute, load_finished_callback);
        job_system.add_job(load_job);

        return models[name];
    }
};  // namespace mag
