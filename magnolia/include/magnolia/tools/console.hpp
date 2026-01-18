#pragma once

#include <functional>

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace console
    {
        b8 initialize();

        void shutdown();

        MAG_API void register_command(const str& command, const std::function<void(const str args)>& func);

        MAG_API void execute_command(const str& command, const str& args);
    };  // namespace console
};      // namespace mag
