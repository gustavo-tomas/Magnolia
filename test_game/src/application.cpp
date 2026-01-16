#include "application.hpp"

#include <magnolia/core/engine.hpp>
#include <magnolia/core/event.hpp>
#include <magnolia/platform/platform.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/threads/job_system.hpp>
#include <magnolia/threads/thread.hpp>

#include "components.hpp"
#include "renderer.hpp"
#include "scene.hpp"

namespace game
{
    TestGame::TestGame()
    {
        mag::EngineInitializeOptions options = {};
        options.window_options.size = {1280, 720};

        MAG_ASSERT(mag::initialize(options), "Failed to initialize mag");

        renderer = mag::create_unique<Renderer>();

        // Set a callback for window events
        mag::window::set_event_callback(BIND_FN(TestGame::on_event));

        set_target_frame_rate(60);

        scene = mag::create_unique<Scene>();

        auto entity_id = scene->get_ecs().create_entity();

        mag::PerspectiveCameraDesc camera_desc = {};
        camera_desc.near = 0.1f;
        camera_desc.far = 100.0f;
        camera_desc.fov = 60.0f;
        camera_desc.viewport_size = mag::window::get_size();
        camera_desc.position = mag::vec3(0.0f);
        camera_desc.rotation = mag::vec3(0.0f);

        mag::PerspectiveCamera camera = mag::PerspectiveCamera(camera_desc);

        TransformComponent transform = {};

        scene->get_ecs().add_component<CameraComponent>(entity_id, camera);
        scene->get_ecs().add_component<TransformComponent>(entity_id, transform);

        scene->on_start();
    }

    TestGame::~TestGame() { mag::shutdown(); }

    void TestGame::run()
    {
        f64 curr_time = 0;
        f64 last_time = 0;
        f64 dt = 0;

        running = true;

        while (running)
        {
            // Calculate dt
            curr_time = mag::plat::get_time();
            dt = (curr_time - last_time) / 1000.0;  // convert from ms to seconds
            last_time = curr_time;

            mag::window::on_update();

            // Skip rendering if minimized or resizing
            if (mag::window::is_minimized())
            {
                mag::thread::sleep(50);
                continue;
            }

            mag::thread::process_callbacks();

            // Update the application
            on_update(dt);

            // Delay if needed
            const f64 delay = (1000.0 / target_frame_rate) - (mag::plat::get_time() - last_time);
            if (delay > 0.0 && target_frame_rate > 0.0)
            {
                mag::thread::sleep(delay);
            }
        }
    }

    void TestGame::on_update(const f32 dt)
    {
        scene->on_update(dt);
        renderer->render_scene(*scene, dt);
    }

    void TestGame::on_event(const mag::Event& e)
    {
        dispatch_event<mag::WindowCloseEvent>(e, BIND_FN(TestGame::on_window_close));
        dispatch_event<mag::QuitEvent>(e, BIND_FN(TestGame::on_quit));

        scene->on_event(e);
        renderer->on_event(e);
    }

    void TestGame::on_quit(const mag::QuitEvent& e)
    {
        (void)e;
        running = false;
    }

    void TestGame::on_window_close(const mag::WindowCloseEvent& e)
    {
        (void)e;
        running = false;
    }

    void TestGame::set_target_frame_rate(const f32 frame_rate) { target_frame_rate = frame_rate; }
};  // namespace game
