#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    using ProcessHandle = u32;

    namespace thread
    {
        b8 initialize_process_manager();

        void shutdown_process_manager();

        // Starts a process. Returns invalid id on error.
        // The user is responsible to end the process.
        MAG_API ProcessHandle start_process(const str& process_path);

        // Kills an existing process.
        MAG_API b8 kill_process(ProcessHandle handle);

        // Checks if a process is running. Invalid process are considered to not be running.
        MAG_API b8 is_process_running(ProcessHandle handle);
    };  // namespace thread
};  // namespace mag
