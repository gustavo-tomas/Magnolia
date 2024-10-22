#include "editor_scene.hpp"

#include "camera_controller.hpp"
#include "core/application.hpp"
#include "core/assert.hpp"
#include "core/file_system.hpp"
#include "core/types.hpp"
#include "ecs/components.hpp"
#include "ecs/ecs.hpp"
#include "threads/job_system.hpp"

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

    EditorScene::~EditorScene() = default;

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

        auto& app = get_application();
        auto& file_watcher = app.get_file_watcher();
        auto& job_system = app.get_job_system();

        std::vector<str> rebuild_dlls;

        const auto& scripts = ecs->get_all_components_of_type<ScriptComponent>();
        for (auto* script : scripts)
        {
            // Rebuild script
            // @NOTE: this uses the python script
            if (file_watcher.was_file_modified(script->file_path))
            {
                rebuild_dlls.push_back(script->file_path);
                file_watcher.reset_file_status(script->file_path);
            }
        }

        if (rebuild_dlls.empty())
        {
            return;
        }

        // Execute python script on another thread
        auto execute = [rebuild_dlls]
        {
            b8 result = true;

            for (const auto& script_file : rebuild_dlls)
            {
                LOG_INFO("Script '{0}' was modified, rebuilding DLL...", script_file);

                // @TODO: cleanup
                str configuration = "debug";
#if defined(MAG_PROFILE)
                configuration = "profile";
#elif defined(MAG_RELEASE)
                configuration = "release";
#endif
                // @TODO: cleanup

                const str rebuild_script = "python3 build.py scripts " + configuration + " " +
                                           std::filesystem::path(script_file).stem().string();
                if (system(rebuild_script.c_str()) == 0)
                {
                    LOG_INFO("Finished rebuilding DLL for '{0}'", script_file);
                }

                else
                {
                    LOG_ERROR("Failed to rebuild DLL for '{0}'", script_file);
                    result = false;
                }
            }

            return result;
        };

        // Callback when finished executing
        auto on_execute_finished = [this](const b8 result)
        {
            // Restart the scene if everything went ok
            if (result && is_running())
            {
                this->on_stop();
                this->on_start();
            }
        };

        Job load_job = Job(execute, on_execute_finished);
        job_system.add_job(load_job);
    }

    void EditorScene::on_component_added(const u32 id, Component* component)
    {
        (void)id;

        // Add script file to file watcher if component is a script
        const auto* script_component = dynamic_cast<ScriptComponent*>(component);
        if (script_component)
        {
            auto& file_watcher = get_application().get_file_watcher();
            file_watcher.watch_file(script_component->file_path);
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

    Camera& EditorScene::get_camera()
    {
        if (is_running())
        {
            auto components = ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
            for (auto [camera_c, transform] : components)
            {
                return camera_c->camera;
            }

            ASSERT(false, "No runtime camera!");
            return std::get<0>(components[0])->camera;
        }

        else
        {
            return *camera;
        }
    }
};  // namespace mag
