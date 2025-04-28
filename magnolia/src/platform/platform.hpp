#pragma once

#include "core/types.hpp"

namespace mag
{
    namespace plat
    {
        b8 initialize();
        void shutdown();

        // Ms since start
        f64 get_time();
    };  // namespace plat
};      // namespace mag
