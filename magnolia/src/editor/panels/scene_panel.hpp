#pragma once

#include "ecs/ecs.hpp"
#include "imgui.h"

namespace mag
{
    class ScenePanel
    {
        public:
            void render(const ImGuiWindowFlags window_flags, ECS& ecs, u64& selected_entity_id);
    };
};  // namespace mag
