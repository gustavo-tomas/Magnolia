#include "application.hpp"

#include <magnolia/core/application.hpp>
#include <magnolia/core/entry_point.hpp>
#include <magnolia/core/event.hpp>
#include <magnolia/gfx/types.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/project/project.hpp>
#include <magnolia/resources/audio.hpp>
#include <magnolia/resources/font.hpp>
#include <magnolia/resources/material.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/resource.hpp>
#include <magnolia/resources/shader.hpp>
#include <magnolia/resources/texture.hpp>
#include <magnolia/scene/scene.hpp>
#include <magnolia/scripting/scripting_engine.hpp>

#include "renderer.hpp"
#include "scene_serializer.hpp"

mag::Application* mag::create_application()
{
    mag::ApplicationOptions app_options = {};

    // Read config file

    const str config_file_path = "test_game/config.json";

    fs::json config;

    if (fs::read_json_data(config_file_path, config))
    {
        window::WindowOptions& window_options = app_options.window_options;
        gfx::GfxOptions& gfx_options = app_options.gfx_options;

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

    return new game::TestGame(app_options);
}

namespace game
{
    TestGame::TestGame(const mag::ApplicationOptions& options) : Application(options), renderer(new Renderer())
    {
        // Set a callback for window events
        mag::window::set_event_callback(BIND_FN(TestGame::process_event));

        // Set a callback to manage resources
        set_on_resource_loaded_callback(BIND_FN(TestGame::on_resource_loaded));

        // Load the project

        mag::Project project;

        const str project_file_path = "test_game/TestGame.proj.json";
        if (!mag::project::load(project_file_path, project))
        {
            LOG_ERROR("Failed to load project: '{0}'", project_file_path);
            return;
        }

        // Then load starting scene

        scene = mag::create_unique<mag::Scene>();

        const str start_scene_file_path = project.get_asset_dir() / project.get_relative_start_scene_path();
        if (!scene::load(start_scene_file_path, *scene))
        {
            LOG_ERROR("Failed to load start scene: '{0}'", start_scene_file_path);
            return;
        }

        scene->on_start();
    }

    TestGame::~TestGame() = default;

    void TestGame::on_update(const f32 dt)
    {
        // Check for scene swaps
        const str& next_scene_file_path = scene->get_next_scene();
        if (!next_scene_file_path.empty())
        {
            mag::Scene* next_scene = new mag::Scene();
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
        scene->on_event(e);
        renderer->on_event(e);
    }

    void TestGame::on_resource_loaded(const mag::IResource* resource)
    {
        // Upload texture data to the GPU
        if (const mag::TextureResource* texture = dynamic_cast<const mag::TextureResource*>(resource))
        {
        }

        // Upload model data to the GPU
        else if (const mag::ModelResource* model = dynamic_cast<const mag::ModelResource*>(resource))
        {
        }

        // Upload font data to the GPU
        else if (const mag::FontResource* font = dynamic_cast<const mag::FontResource*>(resource))
        {
        }

        else if (const mag::MaterialResource* material = dynamic_cast<const mag::MaterialResource*>(resource))
        {
        }

        else if (const mag::ShaderResource* shader = dynamic_cast<const mag::ShaderResource*>(resource))
        {
        }

        else if (const mag::AudioResource* audio = dynamic_cast<const mag::AudioResource*>(resource))
        {
        }
    }
};  // namespace game
