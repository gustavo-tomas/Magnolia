#pragma once

#include "ecs/ecs.hpp"
#include "imgui.h"

namespace sprout
{
    using namespace mag;

    class PropertiesPanel
    {
        public:
            void render(const ImGuiWindowFlags window_flags, ECS &ecs, const u32 selected_entity_id);
    };
};  // namespace sprout
