#include "application.hpp"

#include <magnolia/core/engine.hpp>
#include <magnolia/core/event.hpp>
#include <magnolia/gfx/types.hpp>
#include <magnolia/platform/file_system.hpp>
#include <magnolia/platform/platform.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/project/project.hpp>
#include <magnolia/resources/audio.hpp>
#include <magnolia/resources/font.hpp>
#include <magnolia/resources/material.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/resource.hpp>
#include <magnolia/resources/shader.hpp>
#include <magnolia/resources/texture.hpp>
#include <magnolia/scripting/scripting_engine.hpp>
#include <magnolia/threads/job_system.hpp>
#include <magnolia/threads/thread.hpp>
#include <magnolia/tools/console.hpp>

#include "renderer.hpp"
#include "scene.hpp"
#include "scene_serializer.hpp"

namespace game
{
    struct GameInitializeOptions
    {
            mag::EngineInitializeOptions engine_options = {};
            f32 target_frame_rate = -1;
    };

    GameInitializeOptions read_application_options(const str& config_file_path)
    {
        GameInitializeOptions app_options = {};

        // Read config file

        mag::fs::json config;

        if (mag::fs::read_json_data(config_file_path, config))
        {
            mag::window::WindowOptions& window_options = app_options.engine_options.window_options;
            mag::gfx::GfxOptions& gfx_options = app_options.engine_options.gfx_options;

            u32 count = 0;
            for (const auto& num : config["WindowSize"])
            {
                if (count >= window_options.size.length()) break;
                window_options.size[count++] = num;
            }

            count = 0;
            for (const auto& num : config["WindowPosition"])
            {
                if (count >= window_options.position.length()) break;
                window_options.position[count++] = num;
            }

            count = 0;
            for (const auto& num : config["ScreenResolution"])
            {
                if (count >= gfx_options.resolution.length()) break;
                gfx_options.resolution[count++] = num;
            }

            window_options.title = config["WindowTitle"].get<str>();
            window_options.window_icon = config["WindowIcon"].get<str>();

            app_options.target_frame_rate = config["TargetFrameRate"].get<f32>();
        }

        return app_options;
    }

    TestGame::TestGame()
    {
        const GameInitializeOptions options = read_application_options("test_game/config.json");

        MAG_ASSERT(mag::initialize(options.engine_options), "Failed to initialize mag");

        renderer = mag::create_unique<Renderer>();

        // Set a callback for window events
        mag::window::set_event_callback(BIND_FN(TestGame::on_event));

        set_target_frame_rate(options.target_frame_rate);

        // Load the project

        project = mag::create_unique<mag::Project>();

        const str project_file_path = "test_game/TestGame.proj.json";
        if (!mag::project::load(project_file_path, *project))
        {
            LOG_ERROR("Failed to load project: '{0}'", project_file_path);
            return;
        }

        register_commands();

        // Then load starting scene

        scene = mag::create_unique<Scene>();

        const str start_scene_file_path = project->get_asset_dir() / project->get_relative_start_scene_path();
        if (!scene::load(start_scene_file_path, *scene))
        {
            LOG_ERROR("Failed to load start scene: '{0}'", start_scene_file_path);
            return;
        }

        scene->on_start();
    }

    TestGame::~TestGame()
    {
        scene.reset();
        project.reset();
        mag::shutdown();
    }

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
        // Check for scene swaps
        const str& next_scene_file_path = scene->get_next_scene();
        if (!next_scene_file_path.empty())
        {
            Scene* next_scene = new Scene();
            if (!scene::load(next_scene_file_path, *next_scene))
            {
                LOG_ERROR("Failed to load scene: '{0}'", next_scene_file_path);
                delete next_scene;
            }

            else
            {
                scene.reset(next_scene);
                scene->on_start();
            }

            scene->set_next_scene("");
        }

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

    void TestGame::register_commands()
    {
        mag::console::register_command("recompile_script",
                                       [this](const std::vector<str>& args)
                                       {
                                           for (const str& arg : args)
                                           {
                                               mag::script::RecompileScriptParams params = {};
                                               params.force_recompilation = true;

                                               // We reuse the asset dir retrieved from the project to make things
                                               // easier
                                               params.file_path = project->get_asset_dir() / arg;

                                               mag::script::compile_script(params);

                                               // @TODO: Reload the scene. Ideally we don't want to reload the whole
                                               // thing but this way avoids handling old state.
                                               scene->set_next_scene(scene->get_file_path());
                                           }
                                       });
    }

    void TestGame::set_target_frame_rate(const f32 frame_rate) { target_frame_rate = frame_rate; }
};  // namespace game
