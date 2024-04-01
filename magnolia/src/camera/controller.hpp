#pragma once

#include "camera/camera.hpp"

namespace mag
{
    class Controller
    {
        public:
            void initialize(Camera* camera);
            void shutdown();

            void update(const f32 dt);
            void on_mouse_move(const ivec2& mouse_dir);

        private:
            Camera* camera;
    };
};  // namespace mag
