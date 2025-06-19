#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace script
    {
        // returns nullptr on error
        MAG_API void* load_script(const str& file_path);

        MAG_API void unload_script(void* handle);

        // Skips compilation if scripts exists, or recompile if force recompilation is true
        MAG_API b8 recompile_script(const str& file_path, const b8 force_recompilation = false);

        // returns nullptr on error
        MAG_API void* get_symbol(void* handle, const str& name);
    };  // namespace script
};      // namespace mag
