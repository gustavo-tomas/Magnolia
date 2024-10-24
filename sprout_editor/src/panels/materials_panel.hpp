#pragma once

#include "core/types.hpp"

typedef int ImGuiWindowFlags;

namespace mag
{
    class ECS;
};

namespace sprout
{
    using namespace mag;

    class MaterialsPanel
    {
        public:
            MaterialsPanel();
            ~MaterialsPanel();

            void render(const ImGuiWindowFlags window_flags, ECS& ecs, const u32 selected_entity_id);
    };
};  // namespace sprout
