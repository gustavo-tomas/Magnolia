#include "scene/scene.hpp"

#include <filesystem>

#include "core/application.hpp"
#include "core/logger.hpp"

namespace mag
{
    void Scene::initialize()
    {
        // @TODO: temp hardcoding inside for now
        auto& window = get_application().get_window();

        // Create a render pass
        render_pass.initialize(window.get_size());
        LOG_SUCCESS("RenderPass initialized");

        // Create a camera
        camera.initialize({-100.0f, 5.0f, 0.0f}, {0.0f, 90.0f, 0.0f}, 60.0f, window.get_size(), 0.1f, 10000.0f);
        LOG_SUCCESS("Camera initialized");

        // Create a camera controller for editor
        camera_controller.initialize(&camera);
        LOG_SUCCESS("CameraController initialized");

        get_render_pass().set_camera();

        // @TODO: temp load assets
        cube.initialize();

        cube.get_model().translation = vec3(0, 10, 0);
        cube.get_model().scale = vec3(10);

        models.push_back(cube.get_model());
        render_pass.add_model(cube.get_model());
    }

    void Scene::shutdown()
    {
        cube.shutdown();

        this->camera_controller.shutdown();
        LOG_SUCCESS("EditorController destroyed");

        this->camera.shutdown();
        LOG_SUCCESS("Camera destroyed");

        this->render_pass.shutdown();
        LOG_SUCCESS("RenderPass destroyed");
    }

    void Scene::update(const f32 dt)
    {
        auto& app = get_application();
        auto& model_loader = app.get_model_loader();
        auto& window = app.get_window();

        camera_controller.update(dt);

        // @TODO: testing
        if (window.is_key_down(SDLK_UP))
            render_pass.set_render_scale(render_pass.get_render_scale() + 0.15f * dt);

        else if (window.is_key_down(SDLK_DOWN))
            render_pass.set_render_scale(render_pass.get_render_scale() - 0.15f * dt);
        // @TODO: testing

        // Load enqueued models
        while (!models_queue.empty())
        {
            const str& model_path = models_queue.front();
            const auto model = model_loader.load(model_path);

            models.push_back(*model);
            this->render_pass.add_model(*model);

            models_queue.erase(models_queue.begin());
        }
    }

    void Scene::on_event(Event& e) { camera_controller.on_event(e); }

    void Scene::add_model(const str& path)
    {
        auto& model_loader = get_application().get_model_loader();

        // First check if the path exists
        if (!std::filesystem::exists(path))
        {
            LOG_ERROR("File not found: {0}", path);
            return;
        }

        // Then check if its a directory
        if (std::filesystem::is_directory(path))
        {
            LOG_ERROR("Path is a directory: {0}", path);
            return;
        }

        // Then check if assimp supports this extension
        const std::filesystem::path file_path(path);
        const str extension = file_path.extension().c_str();
        if (!model_loader.is_extension_supported(extension))
        {
            LOG_ERROR("Extension not supported: {0}", extension);
            return;
        }

        // Finally enqueue the model
        models_queue.push_back(path);
    }
};  // namespace mag
