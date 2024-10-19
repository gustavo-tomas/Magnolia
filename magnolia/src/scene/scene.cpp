#include "scene/scene.hpp"

#include "core/application.hpp"
#include "renderer/test_model.hpp"
#include "scene/scriptable_entity.hpp"
#include "scripting/scripting_engine.hpp"

namespace mag
{
    Scene::Scene() : name("Untitled"), ecs(new ECS()) {}

    // @TODO: finish scripting
    const char* script_dll = "build/linux/scripting/libscripting_debug.so";

    NativeScriptComponent* loadScript(const str& path)
    {
        void* handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!handle)
        {
            LOG_ERROR("Failed to load script '{0}': {1}", path, dlerror());
            return nullptr;
        }

        using CreateScriptFunc = ScriptableEntity* (*)();
        using DestroyScriptFunc = void (*)(ScriptableEntity*);

        CreateScriptFn createScript = reinterpret_cast<CreateScriptFunc>(dlsym(handle, "create_script"));
        DestroyScriptFunc destroyScript = reinterpret_cast<DestroyScriptFunc>(dlsym(handle, "destroy_script"));

        if (!createScript || !destroyScript)
        {
            LOG_ERROR("Failed to load script symbols '{0}': {1}", path, dlerror());
            dlclose(handle);
            return nullptr;
        }

        NativeScriptComponent* nsc = new NativeScriptComponent(path, createScript, destroyScript);
        nsc->handle = handle;

        return nsc;
    }

    void Scene::on_start()
    {
        // @TODO: finish scripting
        static b8 added = false;
        if (!added)
        {
            ecs->add_component(0, loadScript(script_dll));
            added = true;
        }

        on_start_internal();

        auto& app = get_application();
        auto& physics_engine = app.get_physics_engine();

        physics_engine.on_simulation_start(this);

        LuaScriptingEngine::new_state();

        // Instantiate scripts
        for (const u32 id : ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            auto* sc = ecs->get_component<ScriptComponent>(id);
            if (!sc->instance)
            {
                sc->instance = new Script();
                sc->instance->ecs = ecs.get();
                sc->instance->entity_id = id;

                LuaScriptingEngine::load_script(sc->file_path);
                LuaScriptingEngine::register_entity(*sc);

                sc->instance->on_create(*sc->instance);
            }
        }

        // Instantiate native scripts
        for (const u32 id : ecs->get_entities_with_components_of_type<NativeScriptComponent>())
        {
            auto* nsc = ecs->get_component<NativeScriptComponent>(id);
            if (!nsc->entity)
            {
                nsc->entity = nsc->create_entity();
                nsc->entity->ecs = ecs.get();
                nsc->entity->entity_id = id;
                nsc->entity->on_create();
            }
        }

        running = true;
    }

    void Scene::on_stop()
    {
        auto& app = get_application();
        auto& physics_engine = app.get_physics_engine();

        // Destroy scripts
        for (const u32 id : ecs->get_entities_with_components_of_type<ScriptComponent>())
        {
            auto* script = ecs->get_component<ScriptComponent>(id);
            script->instance->on_destroy(*script->instance);
            delete script->instance;
            script->instance = nullptr;
        }

        // Destroy native scripts
        for (auto nsc : ecs->get_all_components_of_type<NativeScriptComponent>())
        {
            if (nsc->entity)
            {
                nsc->entity->on_destroy();
                nsc->destroy_entity(nsc->entity);
                nsc->entity = nullptr;
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

        // Set camera positions the same as the transform
        // @TODO: there should be only one value for the camera position. I feel that data duplication
        // could cause issues in the future.

        auto components = ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
        for (auto [camera_c, transform] : components)
        {
            camera_c->camera.set_position(transform->translation);
            camera_c->camera.set_rotation(transform->rotation);
        }

        // Update scripts
        for (auto script : ecs->get_all_components_of_type<ScriptComponent>())
        {
            if (script->instance)
            {
                script->instance->on_update(*script->instance, dt);
            }
        }

        // Update native scripts
        for (auto nsc : ecs->get_all_components_of_type<NativeScriptComponent>())
        {
            if (nsc->entity)
            {
                nsc->entity->on_update(dt);
            }
        }

        on_update_internal(dt);
    }

    void Scene::on_event(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowResizeEvent>(BIND_FN(Scene::on_resize));

        on_event_internal(e);
    }

    void Scene::on_resize(WindowResizeEvent& e)
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
