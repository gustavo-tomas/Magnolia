#include "scene/scene.hpp"

#include "camera/camera.hpp"
#include "core/application.hpp"
#include "core/assert.hpp"
#include "core/event.hpp"
#include "physics/physics.hpp"
#include "renderer/test_model.hpp"
#include "resources/image.hpp"
#include "scene/scriptable_entity.hpp"
#include "scripting/scripting_engine.hpp"

namespace mag
{
    Scene::Scene() : name("Untitled"), ecs(new ECS(10'000, BIND_FN2(Scene::on_component_added))) {}

    Scene::~Scene()
    {
        if (running)
        {
            on_stop();
        }
    }

    void Scene::on_start()
    {
        on_start_internal();

        auto& app = get_application();
        auto& physics_engine = app.get_physics_engine();

        physics_engine.on_simulation_start(this);

        // Instantiate scripts
        for (const u32 id : ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            auto* script = ecs->get_component<ScriptComponent>(id);
            if (!script->entity)
            {
                void* handle = ScriptingEngine::load_script(script->file_path);
                if (!handle)
                {
                    continue;
                }

                void* raw_create_script_fn = ScriptingEngine::get_symbol(handle, "create_script");
                void* raw_destroy_script_fn = ScriptingEngine::get_symbol(handle, "destroy_script");

                if (!raw_create_script_fn || !raw_destroy_script_fn)
                {
                    continue;
                }

                using CreateScriptFnPtr = ScriptableEntity* (*)();
                using DestroyScriptFnPtr = void (*)(ScriptableEntity*);

                CreateScriptFn create_script_fn = reinterpret_cast<CreateScriptFnPtr>(raw_create_script_fn);
                DestroyScriptFn destroy_script_fn = reinterpret_cast<DestroyScriptFnPtr>(raw_destroy_script_fn);

                script->handle = handle;
                script->create_entity = create_script_fn;
                script->destroy_entity = destroy_script_fn;

                script->entity = script->create_entity();
                script->entity->ecs = ecs.get();
                script->entity->entity_id = id;
                script->entity->on_create();
            }
        }

        running = true;
    }

    void Scene::on_stop()
    {
        auto& app = get_application();
        auto& physics_engine = app.get_physics_engine();

        // Destroy scripts
        for (auto script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            if (script->entity)
            {
                script->entity->on_destroy();
                script->destroy_entity(script->entity);
                script->entity = nullptr;

                ScriptingEngine::unload_script(script->handle);
                script->handle = nullptr;
            }
        }

        physics_engine.on_simulation_end();

        on_stop_internal();

        running = false;
    }

    void Scene::on_update(const f32 dt)
    {
        // Delete enqueued entities
        for (const auto& entity : entity_deletion_queue)
        {
            ecs->erase_entity(entity);
        }

        entity_deletion_queue.clear();

        // Update scripts
        for (auto script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            if (script->entity)
            {
                script->entity->on_update(dt);
            }
        }

        on_update_internal(dt);
    }

    void Scene::on_component_added(const u32 id, Component* component)
    {
        (void)id;
        (void)component;
    }

    void Scene::on_event(const Event& e)
    {
        dispatch_event<WindowResizeEvent>(e, BIND_FN(Scene::on_resize));

        // Emit events to the native scripts
        for (auto script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            if (script->entity)
            {
                script->entity->on_event(e);
            }
        }

        on_event_internal(e);
    }

    void Scene::on_resize(const WindowResizeEvent& e)
    {
        const uvec2& size = {e.width, e.height};

        for (auto camera_c : ecs->get_all_components_of_type<CameraComponent>())
        {
            camera_c->camera.set_aspect_ratio(size);
        }
    }

    void Scene::add_model(const str& path)
    {
        auto& app = get_application();
        auto& model_manager = app.get_model_manager();

        const auto model = model_manager.get(path);

        const auto entity = ecs->create_entity();
        ecs->add_component(entity, new TransformComponent());
        ecs->add_component(entity, new ModelComponent(model));
    }

    void Scene::add_sprite(const str& path)
    {
        auto& app = get_application();
        auto& texture_manager = app.get_texture_manager();

        const auto sprite = texture_manager.get(path);

        const auto entity = ecs->create_entity();
        ecs->add_component(entity, new SpriteComponent(sprite, path));
        ecs->add_component(entity, new TransformComponent());
    }

    void Scene::remove_entity(const u32 id)
    {
        if (!ecs->entity_exists(id))
        {
            return;
        }

        // Enqueue entity
        entity_deletion_queue.push_back(id);
    }

    void Scene::set_name(const str& name) { this->name = name; }

    b8 Scene::is_running() const { return running; }

    const str& Scene::get_name() const { return name; }

    ECS& Scene::get_ecs() { return *ecs; }

    Camera& Scene::get_camera()
    {
        // @TODO: for now we assume the active camera is the first entity with a camera component
        auto components = ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
        for (auto [camera_c, transform] : components)
        {
            return camera_c->camera;
        }

        ASSERT(false, "No runtime camera!");
        return std::get<0>(components[0])->camera;
    }

    void Scene::on_start_internal() {}
    void Scene::on_stop_internal() {}
    void Scene::on_event_internal(const Event& e) { (void)e; }
    void Scene::on_update_internal(const f32 dt) { (void)dt; }

    // @TODO
    // void Scene::set_render_scale(const f32 new_render_scale)
    // {
    //     (void)
    //     // render_scale = std::clamp(new_render_scale, 0.01f, 1.0f);

    //     // auto e = WindowResizeEvent(draw_size.x, draw_size.y);
    //     // on_resize(e);
    //     // // build_render_graph(draw_size);
    // }
};  // namespace mag
