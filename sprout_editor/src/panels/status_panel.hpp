#pragma once

#include "imgui.h"

namespace sprout
{
    class StatusPanel
    {
        public:
            StatusPanel();
            ~StatusPanel();

            void render(const ImGuiWindowFlags window_flags);
    };
};  // namespace sprout
