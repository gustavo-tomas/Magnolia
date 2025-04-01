#pragma once

#include "core/types.hpp"

namespace mag
{
    namespace script
    {
        // returns nullptr on error
        void* load_script(const str& file_path);

        void unload_script(void* handle);

        // returns nullptr on error
        void* get_symbol(void* handle, const str& name);
    };  // namespace script
};      // namespace mag
