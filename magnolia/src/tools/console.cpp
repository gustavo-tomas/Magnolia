#include "magnolia/tools/console.hpp"

#include <unordered_map>

#include "magnolia/core/logger.hpp"

namespace mag
{
    namespace console
    {
        struct ConsoleState
        {
                std::unordered_map<str, std::function<void(const str&)>> commands;
        };

        static ConsoleState* state = nullptr;

        b8 initialize()
        {
            state = new ConsoleState();

            return state != nullptr;
        }

        void shutdown() { delete state; }

        void register_command(const str& command, const std::function<void(const str args)>& func)
        {
            if (!state->commands.contains(command))
            {
                state->commands[command] = std::move(func);
                return;
            }

            LOG_WARNING("Command '{0}' is already registered", command);
        }

        void execute_command(const str& command, const str& args)
        {
            auto it = state->commands.find(command);
            if (it != state->commands.end())
            {
                it->second(args);
                return;
            }

            LOG_ERROR("Command '{0}' is not registered", command);
        }
    };  // namespace console
};      // namespace mag
