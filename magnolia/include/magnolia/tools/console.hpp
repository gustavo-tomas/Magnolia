#pragma once

#include <functional>

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace console
    {
        b8 initialize();

        void shutdown();

        MAG_API void register_command(const str& command, const std::function<void(const std::vector<str>&)>&& func);

        MAG_API void execute_command(const str& command, const std::vector<str>& args);

        void on_update();

        void on_event(const void* event);

        u32 get_window_id();
    };  // namespace console
};  // namespace mag
