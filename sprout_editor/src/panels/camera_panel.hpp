#pragma once

#include "camera/camera.hpp"
#include "imgui.h"

namespace mag
{
    class CameraPanel
    {
        public:
            void render(const ImGuiWindowFlags window_flags, Camera& camera);
    };
};  // namespace mag