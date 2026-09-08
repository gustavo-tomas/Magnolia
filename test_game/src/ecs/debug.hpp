#pragma once

#include <magnolia/core/types.hpp>

namespace game
{
    class Scene;

    void initialize_debug_system();
    void shutdown_debug_system();
    void debug_system(Scene& scene, const f32 dt);
};  // namespace game
