#pragma once

#include "core/types.hpp"

namespace mag
{
    class ScriptingEngine
    {
        public:
            static void* load_script(const str& file_path);  // returns nullptr on error
            static void unload_script(void* handle);
            static void* get_symbol(void* handle, const str& name);  // returns nullptr on error
    };
};  // namespace mag
