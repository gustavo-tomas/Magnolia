#pragma once

#include "core/logger.hpp"
#include "core/types.hpp"

namespace mag
{
    class ScriptingEngine
    {
        public:
            static void initialize();

            static void shutdown();

            static void load_script(const str& file_path);

            static void run_script(const str& file_path);
    };
};  // namespace mag
