#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace plat
    {
        b8 initialize();
        void shutdown();

        // Ms since start
        MAG_API f64 get_time();
    };  // namespace plat
};      // namespace mag
