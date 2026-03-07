#include "magnolia/threads/process_manager.hpp"

#include <unordered_map>

#include "magnolia/core/logger.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/platform/file_system.hpp"

#if MAG_PLATFORM_LINUX
    #include <sys/wait.h>
#elif MAG_PLATFORM_WINDOWS
    #error "@TODO: windows process manager not implemented"
#endif

namespace mag
{
    namespace thread
    {
        struct Process
        {
                pid_t pid = -1;
                str path;
        };

        struct ProcessManagerState
        {
                std::unordered_map<ProcessHandle, Process> processes;
                ProcessHandle handle_counter = 0;
        };

        static ProcessManagerState* state = nullptr;

        b8 initialize_process_manager()
        {
            state = new ProcessManagerState();

            return state != nullptr;
        }

        void shutdown_process_manager() { delete state; }

        static ProcessHandle create_handle() { return state->handle_counter++; }

        ProcessHandle start_process(const str& process_path)
        {
            if (!fs::exists(process_path))
            {
                LOG_ERROR("Process executable not found: '{0}'", process_path);
                return Invalid_ID;
            }

            const i32 pid = fork();
            if (pid < 0)
            {
                LOG_ERROR("Fork failed when creating the process: '{0}'", process_path);
                return Invalid_ID;
            }

            // Child process
            if (pid == 0)
            {
                // Child process
                execl(process_path.c_str(), process_path.c_str(), Invalid_ID);

                // If execl returns, it failed
                exit(EXIT_FAILURE);
            }

            // Parent process
            Process process = {};
            process.pid = pid;
            process.path = process_path;

            const ProcessHandle handle = create_handle();

            state->processes[handle] = process;

            return handle;
        }

        b8 kill_process(const ProcessHandle handle)
        {
            auto it = state->processes.find(handle);
            if (it == state->processes.end())
            {
                return false;
            }

            const Process& process = it->second;

            state->processes.erase(it);

            // Try to end the process
            if (process.pid < 0 || kill(process.pid, SIGKILL) != 0)
            {
                LOG_ERROR("Failed to terminate process: PID: {0} - Path: '{1}'", process.pid, process.path);
                return false;
            }

            // Wait for the process to be fully terminated
            i32 status = -1;
            if (waitpid(process.pid, &status, 0) < 0)
            {
                LOG_ERROR("Error when killing process: Path: '{0}' - Error: {1}", process.path, errno);
                return false;
            }

            return true;
        }

        b8 is_process_running(const ProcessHandle handle)
        {
            auto it = state->processes.find(handle);
            if (it == state->processes.end())
            {
                return false;
            }

            const Process& process = it->second;

            // Send signal 0 to check if process exists
            if (kill(process.pid, 0) != 0)
            {
                // Process doesn't exist anymore
                return false;
            }

            // Check if its a zombie process
            i32 status = -1;
            const i32 result = waitpid(process.pid, &status, WNOHANG);

            // Process has terminated
            if (result == process.pid)
            {
                return false;
            }

            // Process is still running
            if (result == 0)
            {
                return true;
            }

            // Error occurred
            LOG_ERROR("Error during process running check: Path: '{0}' - Error: {1}", process.path, errno);
            return false;
        }
    };  // namespace thread
};  // namespace mag
