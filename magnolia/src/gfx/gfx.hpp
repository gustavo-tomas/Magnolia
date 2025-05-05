#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    namespace gfx
    {
        b8 initialize();
        void shutdown();

        void on_update(const f32 dt);
    };  // namespace gfx
};      // namespace mag
