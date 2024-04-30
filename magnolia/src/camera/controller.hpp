#pragma once

#include "camera/camera.hpp"
#include "core/event.hpp"

namespace mag
{
    // Keep this small and flexible
    class CameraController
    {
        public:
            virtual void initialize(Camera* camera) { this->camera = camera; };
            virtual void shutdown() { this->camera = nullptr; };

            virtual void update(const f32 dt) { (void)dt; };
            virtual void on_event(Event& e) { (void)e; };

        protected:
            Camera* camera = nullptr;
    };

    // @TODO: legacy event handling. update before use
    class RuntimeController : public CameraController
    {
        public:
            virtual void update(const f32 dt) override;
            void on_mouse_move(const ivec2& mouse_dir);
    };

    class EditorController : public CameraController
    {
        public:
            virtual void on_event(Event& e) override;

        private:
            void on_mouse_move(MouseMoveEvent& e);
            void on_mouse_scroll(MouseScrollEvent& e);
    };
};  // namespace mag
