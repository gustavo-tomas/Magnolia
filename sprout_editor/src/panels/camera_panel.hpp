#pragma once

typedef int ImGuiWindowFlags;

namespace mag
{
    class Camera;
};

namespace sprout
{
    using namespace mag;

    class CameraPanel
    {
        public:
            CameraPanel();
            ~CameraPanel();

            void render(const ImGuiWindowFlags window_flags, Camera& camera);
    };
};  // namespace sprout
