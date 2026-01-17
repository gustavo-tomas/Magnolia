#include "magnolia/tools/console.hpp"

#include <unordered_map>

#include "magnolia/core/logger.hpp"

namespace mag
{
    namespace console
    {
        struct ConsoleState
        {
                CommandID current_id = 0;
                std::unordered_map<CommandID, std::function<void(const str&)>> commands;
        };

        static ConsoleState* state = nullptr;

        b8 initialize()
        {
            state = new ConsoleState();

            return state != nullptr;
        }

        void shutdown() { delete state; }

        CommandID register_command(const std::function<void(const str args)>& func)
        {
            const CommandID command_id = state->current_id;
            state->commands[command_id] = std::move(func);
            state->current_id++;

            return command_id;
        }

        void execute_command(const CommandID command_id, const str& args)
        {
            auto it = state->commands.find(command_id);
            if (it != state->commands.end())
            {
                it->second(args);
            }

            else
            {
                LOG_ERROR("Command '{0}' is not registered", command_id);
            }
        }
    };  // namespace console
};      // namespace mag
