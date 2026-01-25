#pragma once

#include <vector>

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace script
    {
        // returns nullptr on error
        MAG_API void* load_script(const str& file_path);

        MAG_API void unload_script(void* handle);

        struct RecompileScriptParams
        {
                str file_path;
                b8 force_recompilation = false;
                str compilation_flags = "-std=c++23 -fPIC -shared -O0";
                std::vector<str> include_paths = {"magnolia/include", "libs/glm"};
                std::vector<str> lib_paths = {MAG_BUILD_DIR_BIN "magnolia"};
                std::vector<str> link_libs = {"magnolia"};
                std::vector<str> defines = {"MAG_CONFIG_DEBUG", "MAG_ASSERTIONS_ENABLED=1", "MAG_PROFILE_ENABLED=1",
                                            "GLM_ENABLE_EXPERIMENTAL"};
        };

        // Skips compilation if scripts exists, or recompile if force recompilation is true
        MAG_API b8 compile_script(const RecompileScriptParams& params);

        // returns nullptr on error
        MAG_API void* get_symbol(void* handle, const str& name);
    };  // namespace script
};  // namespace mag
