#include "magnolia/threads/process_manager.hpp"

#include "magnolia/core/logger.hpp"
#include "magnolia/core/types.hpp"

#if MAG_PLATFORM_LINUX
    #include <sys/wait.h>
#elif MAG_PLATFORM_WINDOWS
    #error "@TODO: windows process manager not implemented"
#endif

namespace mag
{
    namespace thread
    {
        struct ProcessManagerState
        {
        };

        static ProcessManagerState* state = nullptr;

        b8 initialize_process_manager()
        {
            state = new ProcessManagerState();

            return state != nullptr;
        }

        void shutdown_process_manager() { delete state; }

        b8 execute_process(const str& process_path, std::vector<str> args)
        {
            const i32 pid = fork();
            if (pid < 0)
            {
                LOG_ERROR("Fork failed when creating the process: '{0}'", process_path);
                return false;
            }

            // Child process
            if (pid == 0)
            {
                std::vector<c8*> argv;
                argv.reserve(args.size());

                for (str& arg : args)
                {
                    argv.push_back(arg.data());
                }

                argv.push_back(nullptr);  // argv must be null-terminated

                // Child process
                const i32 res = execvp(process_path.c_str(), argv.data());

                // If execv returns, it failed. Kill the child.
                exit(res);
            }

            // Parent process
            else
            {
                // Wait for process to finish
                i32 status = 0;
                waitpid(pid, &status, 0);

                if (WIFEXITED(status))
                {
                    const i32 code = WEXITSTATUS(status);
                    return code == 0;
                }

                // Life sucks
                return false;
            }
        }
    };  // namespace thread
};  // namespace mag
