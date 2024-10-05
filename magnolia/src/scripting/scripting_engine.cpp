#include "scripting/scripting_engine.hpp"

#include "core/application.hpp"
#include "scene/scriptable_entity.hpp"
#include "scripting/lua_bindings.hpp"
#include "sol/sol.hpp"

namespace mag
{
#define GET_ENTITY_NAME(entity_id) "entity" + std::to_string(entity_id)

    // Here we assume that the sol state is persistent until a level or stage is over. There is no way to copy sol state
    // between two objects, so we have to re-register the core types and methods every time a transition happens :(

    struct State
    {
            std::set<str> loaded_scripts;
            unique<sol::state> lua;
    };

    static State* state = nullptr;

    void ScriptingEngine::initialize() { state = new State(); }

    void ScriptingEngine::shutdown() { delete state; }

    // Warning! this unloads all registered scripts!
    void ScriptingEngine::new_state()
    {
        state->lua.reset();
        state->loaded_scripts.clear();

        state->lua = create_unique<sol::state>();

        auto& lua = *state->lua;

        lua.open_libraries(sol::lib::base);

        // Register types
        lua.new_usertype<Script>("Script", sol::constructors<Script()>(),

                                 "get_name", &Script::get_component<NameComponent>,

                                 "get_transform", &Script::get_component<TransformComponent>,

                                 "get_camera", &Script::get_component<CameraComponent>,

                                 "entity_id", &Script::entity_id,

                                 "ecs", &Script::ecs);

        lua.new_usertype<NameComponent>("NameComponent", sol::constructors<NameComponent(const str&)>(),

                                        "name", &NameComponent::name);

        lua.new_usertype<TransformComponent>("TransformComponent", sol::constructors<TransformComponent()>(),

                                             "translation", &TransformComponent::translation,

                                             "rotation", &TransformComponent::rotation,

                                             "scale", &TransformComponent::scale);

        lua.new_usertype<CameraComponent>("CameraComponent", sol::constructors<CameraComponent(const Camera&)>(),

                                          "camera", &CameraComponent::camera);

        lua.new_usertype<Camera>(
            "Camera", sol::constructors<Camera(const vec3&, const vec3&, const f32, const f32, const f32, const f32)>(),

            "get_position", &Camera::get_position,

            "set_position", &Camera::set_position);

        LuaBindings::create_lua_bindings(*state->lua);
    }

    void ScriptingEngine::load_script(const str& file_path)
    {
        if (state->loaded_scripts.contains(file_path)) return;

        auto& app = get_application();
        auto& file_system = app.get_file_system();

        Buffer buffer;
        if (!file_system.read_binary_data(file_path, buffer))
        {
            return;
        }

        auto& lua = *state->lua;

        auto res = lua.load_buffer(buffer.cast<const char>(), buffer.get_size());
        if (!res.valid())
        {
            LOG_ERROR("Invalid script: '{0}'", file_path);
            return;
        }

        // Run the loaded script
        sol::function script = res;
        script();

        state->loaded_scripts.insert(file_path);
    }

    void ScriptingEngine::register_entity(const ScriptComponent& sc)
    {
        auto& lua = *state->lua;

        lua[GET_ENTITY_NAME(sc.instance->entity_id)] = sc.instance;
        lua[GET_ENTITY_NAME(sc.instance->entity_id)]["ecs"] = sc.instance->ecs;
        lua[GET_ENTITY_NAME(sc.instance->entity_id)]["entity_id"] = sc.instance->entity_id;

        // Make methods acessible to the rest of the application
        sc.instance->on_create = lua["on_create"];
        sc.instance->on_destroy = lua["on_destroy"];
        sc.instance->on_update = lua["on_update"];
    }
};  // namespace mag
