#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    using ProcessHandle = u32;

    namespace thread
    {
        b8 initialize_process_manager();

        void shutdown_process_manager();

        MAG_API b8 execute_process(const str& process_path, std::vector<str> args);
    };  // namespace thread
};  // namespace mag
