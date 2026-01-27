#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    // Keep this hidden from the user to avoid messing with pid values and allthat
    struct Process;

    namespace thread
    {
        // Starts a process. Returns nullptr on error.
        // The user is responsible to end the process and any allocated memory.
        Process* start_process(const str& process_path);

        // Kills an existing process. The process will be deleted, so its advised to set the variable as null.
        b8 kill_process(Process* process);

        // Checks if a process is running. Invalid process are considered to not be running.
        b8 is_process_running(Process* process);
    };  // namespace thread
};  // namespace mag
