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
#include <magnolia/tools/model_importer.hpp>

#include "renderer.hpp"
#include "scene.hpp"
#include "scene_serializer.hpp"

namespace game
{
    using namespace mag::math;

    TestGame::TestGame()
    {
        const mag::EngineInitializeOptions options = mag::read_config_file("test_game/config.json");

        MAG_ASSERT(mag::initialize(options), "Failed to initialize mag");

        renderer = mag::create_unique<Renderer>();

        // Set a callback for window events
        mag::window::set_event_callback([this](const mag::Event& e) { on_event(e); });

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

        scene = mag::create_unique<Scene>(renderer.get());

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
        running = true;

        while (running)
        {
            mag::window::on_update();

            // Called after window update
            const f32 dt = mag::window::get_delta_time();

            // Skip rendering if minimized or resizing
            if (mag::window::is_minimized())
            {
                mag::thread::sleep(50);
                continue;
            }

            mag::thread::process_callbacks();

            // Update the application
            on_update(dt);
        }
    }

    void TestGame::on_update(const f32 dt)
    {
        // Check for scene swaps
        const str& next_scene_file_path = scene->get_next_scene();
        if (!next_scene_file_path.empty())
        {
            Scene* next_scene = new Scene(renderer.get());
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
        dispatch_event<mag::WindowCloseEvent>(e, [this](const mag::WindowCloseEvent& e) { on_window_close(e); });
        dispatch_event<mag::QuitEvent>(e, [this](const mag::QuitEvent& e) { on_quit(e); });

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

                                               if (mag::script::compile_script(params))
                                               {
                                                   // @TODO: Reload the scene. Ideally we don't want to reload the whole
                                                   // thing but this way avoids handling old state.
                                                   scene->set_next_scene(scene->get_file_path());
                                               }
                                           }
                                       });

        mag::console::register_command("recompile_shader",
                                       [this](const std::vector<str>& args)
                                       {
                                           for (const str& arg : args)
                                           {
                                               // We reuse the asset dir retrieved from the project to make things
                                               // easier
                                               const str file_path = project->get_asset_dir() / arg;

                                               renderer->build_shader(file_path, true);
                                           }
                                       });

        mag::console::register_command("import_model",
                                       [this](const std::vector<str>& args)
                                       {
                                           for (const str& arg : args)
                                           {
                                               mag::ModelImporter importer;

                                               // We reuse the asset dir retrieved from the project to make things
                                               // easier
                                               const str file_path = project->get_asset_dir() / arg;

                                               str out_file_path;
                                               importer.import(file_path, out_file_path);
                                           }
                                       });

        mag::console::register_command("set_fps",
                                       [](const std::vector<str>& args)
                                       {
                                           for (const str& arg : args)
                                           {
                                               const i32 frame_rate = std::stoi(arg);
                                               mag::window::set_target_frame_rate(frame_rate);
                                           }
                                       });
    }
};  // namespace game
