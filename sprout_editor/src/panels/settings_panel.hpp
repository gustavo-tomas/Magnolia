#pragma once

#include "core/types.hpp"
#include "imgui.h"

namespace mag
{
    class SettingsPanel
    {
        public:
            void render(const ImGuiWindowFlags window_flags);

            u32& get_texture_output() { return texture_output; };
            u32& get_normal_output() { return normal_output; };

        private:
            u32 texture_output = 0, normal_output = 0;
    };
};  // namespace mag
