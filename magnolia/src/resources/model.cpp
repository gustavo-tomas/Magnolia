#include "resources/model.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"

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
        auto& model_loader = app.get_model_loader();
        auto& renderer = app.get_renderer();

        Model* model = model_loader.load(name);

        if (model == nullptr)
        {
            LOG_ERROR("Model '{0}' not found, using default", name);

            model = model_loader.load(DEFAULT_MODEL_NAME);
            ASSERT(model, "Default model has not been loaded");
        }

        // Send model data to the GPU
        renderer.add_model(model);

        models[name] = std::shared_ptr<Model>(model);
        return models[name];
    }
};  // namespace mag
