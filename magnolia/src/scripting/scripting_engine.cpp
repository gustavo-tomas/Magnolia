#include "scripting/scripting_engine.hpp"

#include "core/application.hpp"
#include "scene/scriptable_entity.hpp"
#include "sol/sol.hpp"

namespace mag
{
    // Here we assume that the sol state is persistent until a level or stage is over. There is no way to copy sol state
    // between two objects, so we have to re-register the core types and methods every time a transition happens :(

    struct State
    {
            std::unique_ptr<sol::state> lua;
    };

    static State* state = nullptr;

    void ScriptingEngine::initialize() { state = new State(); }

    void ScriptingEngine::shutdown() { delete state; }

    // Warning! this unloads all registered scripts!
    void ScriptingEngine::new_state()
    {
        state->lua.reset();
        state->lua = std::make_unique<sol::state>();

        auto& lua = *state->lua;

        lua.open_libraries(sol::lib::base);

        // Register relevant functions.
        // @NOTE: use pointers when registering app functions to prevent sol2 from freeing the memory when the state is
        // deleted.
        lua.set_function("is_key_down", &Window::is_key_down, &get_application().get_window());
        lua.set_function("is_button_down", &Window::is_button_down, &get_application().get_window());

        // Register key mapping
        // @TODO: finish key mapping (is there an automatic way to do this?)
        lua.new_enum<Keys>("Keys", {{"a", Keys::a}});
        lua.new_enum<Buttons>("Buttons", {{"left", Buttons::Left}});

        // Register types
        lua.new_usertype<ScriptableEntity>("ScriptableEntity", sol::constructors<ScriptableEntity()>(),

                                           //    "on_create", &ScriptableEntity::on_create,

                                           //    "on_destroy", &ScriptableEntity::on_destroy,

                                           //    "on_update", &ScriptableEntity::on_update,

                                           "get_name", &ScriptableEntity::get_component<NameComponent>,

                                           "get_transform", &ScriptableEntity::get_component<TransformComponent>,

                                           "entity_id", &ScriptableEntity::entity_id,

                                           "ecs", &ScriptableEntity::ecs);

        lua.new_usertype<NameComponent>("NameComponent", sol::constructors<NameComponent(const str&)>(),

                                        "name", &NameComponent::name);

        lua.new_usertype<TransformComponent>("TransformComponent", sol::constructors<TransformComponent()>(),

                                             "translation", &TransformComponent::translation,

                                             "rotation", &TransformComponent::rotation,

                                             "scale", &TransformComponent::scale);
    }

    str get_entity_name(const u32 entity_id)
    {
        const str entity_name = "entity" + std::to_string(entity_id);
        return entity_name;
    }

    void ScriptingEngine::load_script(const str& file_path)
    {
        auto& lua = *state->lua;

        auto res = lua.script_file(file_path);

        // @TODO: this probably shouldnt crash the application
        if (!res.valid())
        {
            ASSERT(false, "Invalid script: '{0}'", file_path);
        }
    }

    // @TODO: Move these methods to the component (crash when quitting during runtime)
    void ScriptingEngine::instanciate_script_for_entity(const ECS* ecs, const u32 entity_id)
    {
        auto& lua = *state->lua;

        lua[get_entity_name(entity_id)] = ScriptableEntity();
        lua[get_entity_name(entity_id)]["ecs"] = ecs;
        lua[get_entity_name(entity_id)]["entity_id"] = entity_id;
    }

    // @TODO: This is a particularly ugly solution.
    // This should be called after bind entity, so that the script is instanciated
    void ScriptingEngine::execute_create_method(const u32 entity_id)
    {
        auto& lua = *state->lua;

        std::function<void(ScriptableEntity&)> on_create = lua["on_create"];
        on_create(lua[get_entity_name(entity_id)]);
    }

    void ScriptingEngine::execute_destroy_method(const u32 entity_id)
    {
        auto& lua = *state->lua;

        std::function<void(ScriptableEntity&)> on_destroy = lua["on_destroy"];
        on_destroy(lua[get_entity_name(entity_id)]);
    }

    void ScriptingEngine::execute_update_method(const u32 entity_id, const f32 dt)
    {
        auto& lua = *state->lua;

        std::function<void(ScriptableEntity&, const f32)> on_update = lua["on_update"];
        on_update(lua[get_entity_name(entity_id)], dt);
    }
};  // namespace mag
