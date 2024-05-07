#pragma once

#include "camera/camera.hpp"
#include "core/event.hpp"

namespace mag
{
    // Keep this small and flexible
    class CameraController
    {
        public:
            CameraController(Camera& camera) : camera(camera){};
            virtual ~CameraController() = default;

            virtual void update(const f32 dt) { (void)dt; };
            virtual void on_event(Event& e) { (void)e; };

        protected:
            Camera& camera;
    };

    // @TODO: legacy event handling. update before use
    class RuntimeController : public CameraController
    {
        public:
            virtual void update(const f32 dt) override;
            void on_mouse_move(const ivec2& mouse_dir);
    };

    class EditorCameraController : public CameraController
    {
        public:
            EditorCameraController(Camera& camera) : CameraController(camera){};

            virtual void on_event(Event& e) override;

        private:
            void on_mouse_move(MouseMoveEvent& e);
            void on_mouse_scroll(MouseScrollEvent& e);
    };
};  // namespace mag
