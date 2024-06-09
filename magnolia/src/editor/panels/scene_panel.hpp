#pragma once

#include "ecs/ecs.hpp"
#include "imgui.h"

namespace mag
{
    class ScenePanel
    {
        public:
            void render(const ImGuiWindowFlags window_flags, ECS& ecs);

            u32 get_selected_entity_id() const { return selected_entity_id; };

        private:
            u32 selected_entity_id = INVALID_ID;
    };
};  // namespace mag
