#pragma once

#include "core/event.hpp"

namespace mag
{
    class Camera;
};

namespace sprout
{
    using namespace mag;

    class EditorCameraController
    {
        public:
            EditorCameraController(Camera& camera);
            ~EditorCameraController();

            void on_update(const f32 dt);
            void on_event(const Event& e);

        private:
            void on_mouse_move(const MouseMoveEvent& e);
            void on_mouse_scroll(const MouseScrollEvent& e);

            Camera& camera;
    };
};  // namespace sprout
