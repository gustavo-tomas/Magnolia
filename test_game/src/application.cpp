#include "application.hpp"

#include <core/application.hpp>
#include <core/entry_point.hpp>
#include <core/event.hpp>
#include <project/project.hpp>
#include <renderer/passes/scene_pass.hpp>
#include <renderer/render_graph.hpp>
#include <renderer/renderer.hpp>
#include <scene/scene.hpp>
#include <scene/scene_serializer.hpp>

mag::Application *mag::create_application() { return new game::TestGame("test_game/config.json"); }

namespace game
{
    TestGame::TestGame(const str &config_file_path) : Application(config_file_path)
    {
        mag::Project project;

        const str project_file_path = "test_game/TestGame.proj.json";
        if (!mag::project::load(project_file_path, project))
        {
            LOG_ERROR("Failed to load project: '{0}'", project_file_path);
            return;
        }

        // Then load starting scene

        scene = create_unique<mag::Scene>();

        const str start_scene_file_path = project.get_asset_dir() / project.get_relative_start_scene_path();
        if (!mag::scene::load(start_scene_file_path, *scene))
        {
            LOG_ERROR("Failed to load start scene: '{0}'", start_scene_file_path);
            return;
        }

        mag::Application &app = mag::get_application();
        mag::Window &window = app.get_window();

        build_render_graph(window.get_size());

        scene->on_start();
    }

    TestGame::~TestGame() = default;

    void TestGame::on_update(const f32 dt)
    {
        // Check for scene swaps
        const str &next_scene_file_path = scene->get_next_scene();
        if (!next_scene_file_path.empty())
        {
            mag::Scene *next_scene = new mag::Scene();
            if (!mag::scene::load(next_scene_file_path, *next_scene))
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

        mag::Application &app = mag::get_application();
        mag::Renderer &renderer = app.get_renderer();

        renderer.on_update(*render_graph, *scene);
    }

    void TestGame::on_event(const mag::Event &e)
    {
        scene->on_event(e);

        mag::dispatch_event<mag::WindowResizeEvent>(e,
                                                    [this](const mag::WindowResizeEvent &e) {
                                                        this->build_render_graph({e.width, e.height});
                                                    });
    }

    void TestGame::build_render_graph(const uvec2 &size)
    {
        render_graph.reset(new mag::RenderGraph());

        // @TODO: for now only one output attachment of each type is supported (one color and one depth maximum)
        // @TODO: whatever change is made here has to be copied to the editor (or vice-versa) and this is not good :(

        // mag::DepthPrePass *depth_prepass = new mag::DepthPrePass(size);
        mag::ScenePass *scene_pass = new mag::ScenePass(size);
        mag::PostProcessingPass *post_pass = new mag::PostProcessingPass(size);

        render_graph->set_output_attachment("OutputColor");

        // render_graph->add_pass(depth_prepass);
        render_graph->add_pass(scene_pass);
        render_graph->add_pass(post_pass);

        render_graph->build();
    }
};  // namespace game
