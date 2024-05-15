#include "scene/scene.hpp"

#include <filesystem>

#include "core/application.hpp"
#include "core/logger.hpp"

namespace mag
{
    Scene::Scene()
        : BaseScene(new ECS(),
                    new Camera({-100.0f, 5.0f, 0.0f}, {0.0f, 90.0f, 0.0f}, 60.0f, {800, 600}, 0.1f, 10000.0f),
                    new StandardRenderPass({800, 600})),
          camera_controller(new EditorCameraController(*camera))
    {
        const u32 loops = 4;
        for (u32 i = 0; i < loops; i++)
        {
            for (u32 j = 0; j < loops; j++)
            {
                const str name = "Cube" + std::to_string(i) + std::to_string(j);
                Cube* cube = new Cube(name);

                auto cube_entity = ecs->create_entity(name);
                ecs->add_component(cube_entity, new TransformComponent(vec3(i * 30, 10, j * 30), vec3(0), vec3(10)));
                ecs->add_component(cube_entity, new ModelComponent(cube->get_model()));

                cubes.emplace_back(cube);
                render_pass->add_model(cube->get_model());

                LOG_INFO("Cube created");
            }
        }

        // Light
        for (u32 i = 0; i < LightComponent::MAX_NUMBER_OF_LIGHTS; i++)
        {
            Cube* cube = new Cube("cubeson");

            auto light = ecs->create_entity("Light");
            ecs->add_component(light, new LightComponent());
            ecs->add_component(light, new ModelComponent(cube->get_model()));
            ecs->add_component(light, new TransformComponent(vec3(i * 20, 30, 0)));

            cubes.emplace_back(cube);
            render_pass->add_model(cube->get_model());
        }
    }

    void Scene::update(const f32 dt)
    {
        auto& app = get_application();
        auto& model_loader = app.get_model_loader();
        auto& window = app.get_window();

        camera_controller->update(dt);

        // @TODO: testing
        if (window.is_key_down(SDLK_UP))
            render_pass->set_render_scale(render_pass->get_render_scale() + 0.15f * dt);

        else if (window.is_key_down(SDLK_DOWN))
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
