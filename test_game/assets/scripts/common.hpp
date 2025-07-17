#pragma once

// Common definitions that need to be available to different scripts

#include <magnolia/platform/window.hpp>

#include "../../src/components.hpp"
#include "../../src/scriptable_entity.hpp"

// Enemy damage
struct DamageData
{
        f32 damage = 1.0f;
};
