#pragma once

#include "camera/camera.hpp"
#include "core/event.hpp"

namespace mag
{
    class EditorCameraController
    {
        public:
            EditorCameraController(Camera& camera) : camera(camera) {}
            ~EditorCameraController() = default;

            void on_update(const f32 dt);
            void on_event(Event& e);

        private:
            void on_mouse_move(MouseMoveEvent& e);
            void on_mouse_scroll(MouseScrollEvent& e);

            Camera& camera;
    };
};  // namespace mag
