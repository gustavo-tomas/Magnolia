#pragma once

#include <functional>

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace console
    {
        typedef u16 CommandID;

        b8 initialize();

        void shutdown();

        MAG_API CommandID register_command(const std::function<void(const str args)>& func);

        MAG_API void execute_command(const CommandID command_id, const str& args);
    };  // namespace console
};      // namespace mag
