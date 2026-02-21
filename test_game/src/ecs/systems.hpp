#pragma once

#include <magnolia/core/types.hpp>

namespace game
{
    class Scene;

    void player_system(Scene& scene, const f32 dt);
    void bullet_system(Scene& scene, const f32 dt);
};  // namespace game
