#include "scripting/scripting_engine.hpp"

// @TODO: this is unix only, create an interface for the windows build
#include <dlfcn.h>

#include <filesystem>

#include "core/application.hpp"
#include "core/buffer.hpp"
#include "core/file_system.hpp"
#include "core/logger.hpp"
#include "scene/scriptable_entity.hpp"
#include "scripting/lua_bindings.hpp"
#include "sol/sol.hpp"

namespace mag
{
    void* ScriptingEngine::load_script(const str& file_path)
    {
        // @TODO: cleanup
        str scripts_bin_folder = "scripts/";
        str extension = ".so";
        str configuration = "_debug";
        {
            const std::filesystem::path cwd = std::filesystem::current_path();
            const str last_folder = cwd.filename().string();
            str system = "linux";

#if defined(_WIN32)
            system = "windows";
            extension = ".dll";
#endif

#if defined(MAG_PROFILE)
            configuration = "_profile";
#elif defined(MAG_RELEASE)
            configuration = "_release";
#endif

            if (last_folder == "Magnolia") scripts_bin_folder = "build/" + system + "/" + scripts_bin_folder;
        }
        // @TODO: cleanup

        const str script_src = std::filesystem::path(file_path).stem();
        const str script_dll = scripts_bin_folder + "lib" + script_src + configuration + extension;

        // @TODO: see if we can load this from memory
        void* handle = dlopen(script_dll.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!handle)
        {
            LOG_ERROR("Failed to load script '{0}': {1}", script_dll, dlerror());
            return nullptr;
        }

        return handle;
    }

    void ScriptingEngine::unload_script(void* handle)
    {
        if (handle)
        {
            dlclose(handle);
        }
    }

    void* ScriptingEngine::get_symbol(void* handle, const str& name)
    {
        if (!handle)
        {
            LOG_ERROR("Handle is nullptr");
            return nullptr;
        }

        void* symbol = dlsym(handle, name.c_str());

        if (!symbol)
        {
            LOG_ERROR("Failed to load script symbols '{0}': {1}", name, dlerror());
            return nullptr;
        }

        return symbol;
    }

#define GET_ENTITY_NAME(entity_id) "entity" + std::to_string(entity_id)

    // Here we assume that the sol state is persistent until a level or stage is over. There is no way to copy sol state
    // between two objects, so we have to re-register the core types and methods every time a transition happens :(

    struct State
    {
            std::set<str> loaded_scripts;
            unique<sol::state> lua;
    };

    static State* state = nullptr;

    void LuaScriptingEngine::initialize() { state = new State(); }

    void LuaScriptingEngine::shutdown() { delete state; }

    // Warning! this unloads all registered scripts!
    void LuaScriptingEngine::new_state()
    {
        state->lua.reset();
        state->loaded_scripts.clear();

        state->lua = create_unique<sol::state>();

        auto& lua = *state->lua;

        lua.open_libraries(sol::lib::base);

        // Register types
        lua.new_usertype<LuaScript>("LuaScript", sol::constructors<LuaScript()>(),

                                    "get_name", &LuaScript::get_component<NameComponent>,

                                    "get_transform", &LuaScript::get_component<TransformComponent>,

                                    "get_camera", &LuaScript::get_component<CameraComponent>,

                                    "entity_id", &LuaScript::entity_id,

                                    "ecs", &LuaScript::ecs);

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

    void LuaScriptingEngine::load_script(const str& file_path)
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

        auto res = lua.load_buffer(buffer.cast<const c8>(), buffer.get_size());
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

    void LuaScriptingEngine::register_entity(const LuaScriptComponent& sc)
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
