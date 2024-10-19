#pragma once

#include "core/logger.hpp"
#include "core/types.hpp"

namespace mag
{
    class ScriptComponent;
    class LuaScriptingEngine
    {
        public:
            static void initialize();

            static void shutdown();

            // Warning! this unloads all registered scripts!
            static void new_state();

            static void load_script(const str& file_path);

            static void register_entity(const ScriptComponent& sc);
    };
};  // namespace mag
