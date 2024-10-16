#include "editor_scene.hpp"

#include "core/application.hpp"
#include "core/types.hpp"

namespace mag
{
    EditorScene::EditorScene()
        : Scene(),
          camera(new Camera({0.0f, 50.0f, 150.0f}, vec3(0.0f), 60.0f, 800.0f / 600.0f, 1.0f, 10000.0f)),
          camera_controller(new EditorCameraController(*camera))
    {
        auto& app = get_application();
        auto& window = app.get_window();

        const uvec2 window_size = window.get_size();
        current_viewport_size = window_size;

        camera->set_aspect_ratio(current_viewport_size);
    }

    void EditorScene::on_start_internal()
    {
        // Save current state
        temporary_ecs = create_unique<ECS>(*ecs);
    }

    void EditorScene::on_stop_internal()
    {
        ecs.reset();
        ecs = create_unique<ECS>(*temporary_ecs);
        temporary_ecs.reset();
    }

    void EditorScene::on_update_internal(const f32 dt)
    {
        camera_controller->on_update(dt);

        // Update camera transforms
        auto components = ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
        for (auto [camera_c, transform] : components)
        {
            camera_c->camera.set_position(transform->translation);
            camera_c->camera.set_rotation(transform->rotation);
        }
    }

    void EditorScene::on_event_internal(Event& e) { camera_controller->on_event(e); }

    void EditorScene::on_viewport_resize(const uvec2& new_viewport_size)
    {
        if (new_viewport_size == current_viewport_size)
        {
            return;
        }

        current_viewport_size = new_viewport_size;

        camera->set_aspect_ratio(current_viewport_size);

        for (auto camera_c : ecs->get_all_components_of_type<CameraComponent>())
        {
            camera_c->camera.set_aspect_ratio(current_viewport_size);
        }
    }

    void EditorScene::on_resize(WindowResizeEvent& e)
    {
        // Don't do anything on window resize (we handle the viewport resize only)
        (void)e;
    }
};  // namespace mag
