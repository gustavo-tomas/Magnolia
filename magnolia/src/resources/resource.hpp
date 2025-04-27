#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    namespace resource
    {
        // Initialize all resource subsystems
        b8 initialize();
        void shutdown();
    };  // namespace resource
};      // namespace mag
