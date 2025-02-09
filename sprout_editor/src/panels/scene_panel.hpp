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

    class ScenePanel
    {
        public:
            ScenePanel();
            ~ScenePanel();

            void render(const ImGuiWindowFlags window_flags, ECS& ecs);

            u32 get_selected_entity_id() const;

        private:
            u32 selected_entity_id = Invalid_ID;
    };
};  // namespace sprout
