#include "magnolia/threads/process_manager.hpp"

#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"

#if MAG_PLATFORM_LINUX
    #include <sys/wait.h>
#elif MAG_PLATFORM_WINDOWS
    #error "@TODO: windows process manager not implemented"
#endif

namespace mag
{
    struct Process
    {
            pid_t pid = -1;
            str path;
    };

    namespace thread
    {
        Process* start_process(const str& process_path)
        {
            if (!fs::exists(process_path))
            {
                LOG_ERROR("Process executable not found: '{0}'", process_path);
                return nullptr;
            }

            const i32 pid = fork();
            if (pid < 0)
            {
                LOG_ERROR("Fork failed when creating the process: '{0}'", process_path);
                return nullptr;
            }

            // Child process
            if (pid == 0)
            {
                // Child process
                execl(process_path.c_str(), process_path.c_str(), nullptr);

                // If execl returns, it failed
                exit(EXIT_FAILURE);
            }

            // Parent process
            Process* process = new Process();
            process->pid = pid;
            process->path = process_path;

            return process;
        }

        b8 kill_process(Process* process)
        {
            if (process == nullptr)
            {
                return false;
            }

            const i32 pid = process->pid;
            const str path = process->path;

            delete process;

            // Try to end the process
            if (pid < 0 || kill(pid, SIGKILL) != 0)
            {
                LOG_ERROR("Failed to terminate process: PID: {0} - Path: '{1}'", pid, path);
                return false;
            }

            // Wait for the process to be fully terminated
            i32 status = -1;
            if (waitpid(pid, &status, 0) < 0)
            {
                LOG_ERROR("Error when killing process: Path: '{0}' - Error: {1}", path, errno);
                return false;
            }

            return true;
        }

        b8 is_process_running(Process* process)
        {
            // Send signal 0 to check if process exists
            if ((process == nullptr) || kill(process->pid, 0) != 0)
            {
                // Process doesn't exist anymore
                return false;
            }

            // Check if its a zombie process
            i32 status = -1;
            const i32 result = waitpid(process->pid, &status, WNOHANG);

            // Process has terminated
            if (result == process->pid)
            {
                return false;
            }

            // Process is still running
            if (result == 0)
            {
                return true;
            }

            // Error occurred
            LOG_ERROR("Error during process running check: Path: '{0}' - Error: {1}", process->path, errno);
            return false;
        }
    };  // namespace thread
};  // namespace mag
