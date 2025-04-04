#pragma once

#include "core/types.hpp"

namespace mag
{
    namespace script
    {
        // returns nullptr on error
        void* load_script(const str& file_path);

        void unload_script(void* handle);

        // Skips compilation if scripts exists, or recompile if force recompilation is true
        b8 recompile_script(const str& file_path, const b8 force_recompilation = false);

        // returns nullptr on error
        void* get_symbol(void* handle, const str& name);
    };  // namespace script
};      // namespace mag
