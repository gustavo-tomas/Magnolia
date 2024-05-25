#include "scene/scene.hpp"

#include <filesystem>

#include "core/application.hpp"
#include "core/logger.hpp"

namespace mag
{
    Scene::Scene()
        : BaseScene(new ECS(),
                    new Camera({-200.0f, 50.0f, 0.0f}, {0.0f, 90.0f, 0.0f}, 60.0f, {800, 600}, 1.0f, 10000.0f),
                    new StandardRenderPass({800, 600})),
          camera_controller(new EditorCameraController(*camera)),
          cube(new Cube())
    {
        auto& cube_model = cube->get_model();
        const auto& light_model =
            get_application().get_model_loader().load("magnolia/assets/models/lightbulb/lightbulb.glb");

        const i32 loops = 5;
        u32 count = 0;
        for (i32 i = -loops / 2; i <= loops / 2; i++)
        {
            for (i32 j = -loops / 2; j <= loops / 2; j++)
            {
                const str name = "Cube" + std::to_string(count++);

                const auto cube_entity = ecs->create_entity(name);
                ecs->add_component(cube_entity, new TransformComponent(vec3(i * 30, 10, j * 30), vec3(0), vec3(10)));
                ecs->add_component(cube_entity, new ModelComponent(cube_model));

                render_pass->add_model(cube_model);

                LOG_INFO("Cube created");
            }
        }

        // Light
        for (i32 i = 0; i < static_cast<i32>(LightComponent::MAX_NUMBER_OF_LIGHTS); i++)
        {
            const auto light = ecs->create_entity("Light" + std::to_string(i));
            const vec3 color = vec3(1);
            const f32 intensity = 100;

            ecs->add_component(light, new LightComponent(color, intensity));
            ecs->add_component(light, new ModelComponent(*light_model));
            ecs->add_component(light, new TransformComponent(vec3(500 - (500 * i), 100, 0), vec3(0), vec3(10)));

            render_pass->add_model(*light_model);

            LOG_INFO("Light created");
        }
    }

    void Scene::update(const f32 dt)
    {
        auto& app = get_application();
        auto& model_loader = app.get_model_loader();
        auto& window = app.get_window();

        camera_controller->update(dt);

        // @TODO: testing
        if (window.is_key_down(Key::Up))
            render_pass->set_render_scale(render_pass->get_render_scale() + 0.15f * dt);

        else if (window.is_key_down(Key::Down))
            render_pass->set_render_scale(render_pass->get_render_scale() - 0.15f * dt);
        // @TODO: testing

        // Load enqueued models
        while (!models_queue.empty())
        {
            const str& model_path = models_queue.front();
            const auto model = model_loader.load(model_path);

            auto entity = ecs->create_entity();
            ecs->add_component(entity, new TransformComponent());
            ecs->add_component(entity, new ModelComponent(*model));

            render_pass->add_model(*model);

            models_queue.erase(models_queue.begin());
        }
    }

    void Scene::on_event(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<ViewportResizeEvent>(BIND_FN(Scene::on_viewport_resize));

        camera_controller->on_event(e);
    }

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
