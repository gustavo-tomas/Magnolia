#pragma once

#include "camera/camera.hpp"
#include "imgui.h"

// @TODO: this is temporary-ish. The camera will (should?) be an ECS component.

namespace mag
{
    class CameraPanel
    {
        public:
            void render(const ImGuiWindowFlags window_flags, Camera& camera);
    };
};  // namespace mag
