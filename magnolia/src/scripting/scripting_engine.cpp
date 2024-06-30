#include "scripting/scripting_engine.hpp"

#include <map>

#include "sol/sol.hpp"

namespace mag
{
    struct State
    {
            sol::state lua;
            std::map<str, sol::load_result> scripts;
    };

    static State* state;

    void ScriptingEngine::initialize()
    {
        state = new State();
        state->lua.open_libraries(sol::lib::base);
    }

    void ScriptingEngine::shutdown() { delete state; }

    void ScriptingEngine::load_script(const str& file_path)
    {
        state->scripts[file_path] = state->lua.load_file(file_path);

        // @TODO: this probably shouldnt crash the application
        if (!state->scripts[file_path].valid())
        {
            ASSERT(false, "Invalid script: '{0}'", file_path);
        }
    }

    void ScriptingEngine::run_script(const str& file_path)
    {
        // Execute the script
        state->scripts[file_path]();

        std::function<void()> on_create = state->lua["on_create"];
        std::function<void()> on_destroy = state->lua["on_destroy"];
        std::function<void(f32)> on_update = state->lua["on_update"];

        on_create();
        on_update(0.1f);
        on_destroy();
    }
};  // namespace mag
