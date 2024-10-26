#pragma once

#include "core/types.hpp"
#include "imgui.h"

namespace sprout
{
    using namespace mag;

    class SettingsPanel
    {
        public:
            SettingsPanel();
            ~SettingsPanel();

            void render(const ImGuiWindowFlags window_flags);

            u32& get_texture_output();
            u32& get_normal_output();
            b8& is_bounding_box_enabled();
            b8& is_physics_colliders_enabled();

        private:
            u32 texture_output = 0, normal_output = 0;
            b8 enable_bounding_boxes = true, enable_physics_boxes = true;
    };
};  // namespace sprout
