#pragma once

// Common definitions that need to be available to different scripts

#include <magnolia/ecs/components.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/scene/scriptable_entity.hpp>

// Enemy damage
struct DamageData
{
        f32 damage = 1.0f;
};
