#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace thread
    {
        b8 initialize();
        void shutdown();

        MAG_API u32 get_core_count();

        MAG_API void sleep(f64 ms);
    };  // namespace thread
};  // namespace mag
