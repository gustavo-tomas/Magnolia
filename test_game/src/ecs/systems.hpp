#pragma once

#include <magnolia/core/types.hpp>

namespace game
{
    class Scene;

    void execute_systems(Scene& scene, const f32 dt);

    void player_system(Scene& scene, const f32 dt);
    void bullet_system(Scene& scene, const f32 dt);
    void enemy_system(Scene& scene, const f32 dt);
};  // namespace game
