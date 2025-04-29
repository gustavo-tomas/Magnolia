#pragma once

#include "core/types.hpp"

namespace mag
{
    namespace thread
    {
        b8 initialize();
        void shutdown();

        void sleep(const f64 ms);
    };  // namespace thread
};      // namespace mag
