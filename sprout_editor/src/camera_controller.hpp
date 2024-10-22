#pragma once

#include "core/event.hpp"

namespace mag
{
    class Camera;

    class EditorCameraController
    {
        public:
            EditorCameraController(Camera& camera);
            ~EditorCameraController();

            void on_update(const f32 dt);
            void on_event(Event& e);

        private:
            void on_mouse_move(MouseMoveEvent& e);
            void on_mouse_scroll(MouseScrollEvent& e);

            Camera& camera;
    };
};  // namespace mag
