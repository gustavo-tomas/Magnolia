#pragma once

#include "camera/camera.hpp"
#include "core/window.hpp"

namespace mag
{
    class Controller
    {
        public:
            void initialize(Camera* camera, Window* window);
            void shutdown();

            void update(const f32 dt);
            void on_mouse_move(const ivec2& mouse_dir);

        private:
            Camera* camera;
            Window* window;
    };
};  // namespace mag
