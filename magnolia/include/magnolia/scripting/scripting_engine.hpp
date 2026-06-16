#pragma once

#include <vector>

#include "magnolia/core/types.hpp"

namespace mag
{
    // The scripting engine keeps track of the loaded scripts. All scripts are unloaded during shutdown, but the user
    // has the option to unload manually if they want.

    namespace script
    {
        using ScriptHandle = u32;

        b8 initialize();

        void shutdown();

        // returns invalid on error
        MAG_API ScriptHandle load_script(const str& file_path);

        MAG_API b8 unload_script(const ScriptHandle handle);

        // returns nullptr on error
        MAG_API void* get_symbol(const ScriptHandle handle, const str& name);

        struct RecompileScriptParams
        {
                str file_path;
                b8 force_recompilation = false;
                str compilation_flags = "-std=c++23 -fPIC -shared -O0";
                std::vector<str> include_paths = {"magnolia/include"};
                std::vector<str> lib_paths = {MAG_BUILD_DIR_BIN "magnolia"};
                std::vector<str> link_libs = {"magnolia"};
                std::vector<str> defines = {"MAG_CONFIG_DEBUG", "MAG_ASSERTIONS_ENABLED=1", "MAG_PROFILE_ENABLED=1"};
        };

        // Skips compilation if scripts exists, or recompile if force recompilation is true
        MAG_API b8 compile_script(const RecompileScriptParams& params);
    };  // namespace script
};  // namespace mag
