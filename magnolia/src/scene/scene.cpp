#include "scene/scene.hpp"

#include "core/application.hpp"
#include "scene/scene_pass.hpp"
#include "scene/scriptable_entity.hpp"
#include "scripting/scripting_engine.hpp"

namespace mag
{
    Scene::Scene()
        : name("Untitled"),
          ecs(new ECS()),
          camera(new Camera({-200.0f, 50.0f, 0.0f}, {0.0f, 90.0f, 0.0f}, 60.0f, 800.0f / 600.0f, 1.0f, 10000.0f)),
          camera_controller(new EditorCameraController(*camera)),
          render_graph(new RenderGraph())
    {
        auto& app = get_application();
        auto& window = app.get_window();

        // @TODO: reimplement missing features:
        // - Multisampling
        // - Render scale (just for the scene passes)
        // - Triple buffering

        const uvec2 window_size = window.get_size();
        const uvec2 viewport_size = app.get_editor().get_viewport_size();

        camera->set_aspect_ratio(viewport_size);
        build_render_graph(window_size, viewport_size);
    }

    Scene::~Scene()
    {
        if (runtime_ecs)
        {
            stop_runtime();
        }
    }

    void Scene::update(const f32 dt)
    {
        switch (current_state)
        {
            case SceneState::Editor:
                update_editor(dt);
                break;

            case SceneState::Runtime:
                update_runtime(dt);
                break;

            default:
                break;
        }
    }

    void Scene::start_runtime()
    {
        auto& app = get_application();
        auto& physics_engine = app.get_physics_engine();

        runtime_ecs = std::make_unique<ECS>(*ecs);
        current_state = SceneState::Runtime;

        physics_engine.on_simulation_start();

        ScriptingEngine::new_state();

        // Instantiate the script
        for (const u32 id : runtime_ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            auto* sc = runtime_ecs->get_component<ScriptComponent>(id);
            if (!sc->instance)
            {
                sc->instance = new Script();
                sc->instance->ecs = runtime_ecs.get();
                sc->instance->entity_id = id;

                ScriptingEngine::load_script(sc->file_path);
                ScriptingEngine::register_entity(*sc);

                sc->instance->on_create(*sc->instance);
            }
        }

        for (const u32 id : runtime_ecs->get_entities_with_components_of_type<NativeScriptComponent>())
        {
            auto* nsc = runtime_ecs->get_component<NativeScriptComponent>(id);
            if (!nsc->instance)
            {
                nsc->instance = nsc->instanciate_script();
                nsc->instance->ecs = runtime_ecs.get();
                nsc->instance->entity_id = id;
                nsc->instance->on_create();
            }
        }
    }

    void Scene::stop_runtime()
    {
        auto& app = get_application();
        auto& physics_engine = app.get_physics_engine();

        for (const u32 id : runtime_ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            auto* script = runtime_ecs->get_component<ScriptComponent>(id);
            script->instance->on_destroy(*script->instance);
            delete script->instance;
            script->instance = nullptr;
        }

        for (auto nsc : runtime_ecs->get_all_components_of_type<NativeScriptComponent>())
        {
            nsc->instance->on_destroy();
            nsc->destroy_script(nsc);
        }

        runtime_ecs.reset();
        current_state = SceneState::Editor;

        physics_engine.on_simulation_end();
    }

    void Scene::update_editor(const f32 dt)
    {
        // auto& app = get_application();
        // auto& window = app.get_window();

        // Delete enqueued entities
        for (const auto& entity : editor_deletion_queue)
        {
            ecs->erase_entity(entity);
        }

        editor_deletion_queue.clear();

        camera_controller->update(dt);

        // @TODO: testing
        // if (window.is_key_down(Key::Up))
        //     set_render_scale(render_scale + 0.15f * dt);

        // else if (window.is_key_down(Key::Down))
        //     set_render_scale(render_scale - 0.15f * dt);
        // @TODO: testing
    }

    void Scene::update_runtime(const f32 dt)
    {
        // Set camera positions the same as the transform
        // @TODO: there should be only one value for the camera position. I feel that data duplication
        // could cause issues in the future.

        auto components = runtime_ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
        for (auto [camera_c, transform] : components)
        {
            camera_c->camera.set_position(transform->translation);
            camera_c->camera.set_rotation(transform->rotation);
        }

        // Update scripts
        for (const u32 id : runtime_ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            auto* script = runtime_ecs->get_component<ScriptComponent>(id);
            script->instance->on_update(*script->instance, dt);
        }

        for (auto nsc : runtime_ecs->get_all_components_of_type<NativeScriptComponent>())
        {
            nsc->instance->on_update(dt);
        }
    }

    void Scene::on_event(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowResizeEvent>(BIND_FN(Scene::on_resize));
        dispatcher.dispatch<ViewportResizeEvent>(BIND_FN(Scene::on_viewport_resize));

        // Don't process editor input during runtime
        if (current_state == SceneState::Editor)
        {
            camera_controller->on_event(e);
        }
    }

    void Scene::on_resize(WindowResizeEvent& e)
    {
        const uvec2 window_size = {e.width, e.height};
        const uvec2 viewport_size = get_application().get_editor().get_viewport_size();

        camera->set_aspect_ratio(viewport_size);
        build_render_graph(window_size, viewport_size);
    }

    void Scene::on_viewport_resize(ViewportResizeEvent& e)
    {
        const uvec2 viewport_size = {e.width, e.height};
        const uvec2 window_size = get_application().get_window().get_size();

        camera->set_aspect_ratio(viewport_size);
        build_render_graph(window_size, viewport_size);

        for (auto camera_c : ecs->get_all_components_of_type<CameraComponent>())
        {
            camera_c->camera.set_aspect_ratio(viewport_size);
        }

        if (!runtime_ecs) return;

        for (auto camera_c : runtime_ecs->get_all_components_of_type<CameraComponent>())
        {
            camera_c->camera.set_aspect_ratio(viewport_size);
        }
    }

    void Scene::build_render_graph(const uvec2& size, const uvec2& viewport_size)
    {
        render_graph.reset(new RenderGraph());

        // @TODO: for now only one output attachment of each type is supported (one color and one depth maximum)

        ScenePass* scene_pass = new ScenePass(viewport_size);
        PhysicsPass* physics_pass = new PhysicsPass(viewport_size);
        GridPass* grid_pass = new GridPass(viewport_size);
        EditorPass* editor_pass = new EditorPass(size);

        render_graph->set_output_attachment("EditorOutputColor");
        // render_graph->set_output_attachment("OutputColor");

        render_graph->add_pass(scene_pass);
        render_graph->add_pass(physics_pass);
        render_graph->add_pass(grid_pass);
        render_graph->add_pass(editor_pass);

        render_graph->build();
    }

    // void Scene::set_render_scale(const f32 new_render_scale)
    // {
    //     (void)
    //     // render_scale = std::clamp(new_render_scale, 0.01f, 1.0f);

    //     // auto e = WindowResizeEvent(draw_size.x, draw_size.y);
    //     // on_resize(e);
    //     // // build_render_graph(draw_size);
    // }

    void Scene::add_model(const str& path)
    {
        auto& app = get_application();
        auto& model_manager = app.get_model_manager();

        const auto model = model_manager.load(path);

        const auto entity = ecs->create_entity();
        ecs->add_component(entity, new TransformComponent());
        ecs->add_component(entity, new ModelComponent(model));
    }

    void Scene::remove_entity(const u32 id)
    {
        if (!ecs->entity_exists(id)) return;

        // Enqueue entity
        editor_deletion_queue.push_back(id);
    }
};  // namespace mag
