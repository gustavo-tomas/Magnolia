#pragma once

#include "camera/camera.hpp"

namespace mag
{
    // Keep this small and flexible
    class CameraController
    {
        public:
            virtual void initialize(Camera* camera) { this->camera = camera; };
            virtual void shutdown() { this->camera = nullptr; };

            virtual void update(const f32 dt) { (void)dt; };

        protected:
            Camera* camera = nullptr;
    };

    class RuntimeController : public CameraController
    {
        public:
            virtual void update(const f32 dt) override;
            void on_mouse_move(const ivec2& mouse_dir);
    };

    class EditorController : public CameraController
    {
        public:
            void on_mouse_move(const ivec2& mouse_dir);
    };
};  // namespace mag
