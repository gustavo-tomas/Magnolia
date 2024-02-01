#pragma once

#include "core/window.hpp"

namespace mag
{
    class Editor
    {
        public:
            void initialize(Window& window);
            void shutdown();
            void update();

        private:
            Window* window;
    };
};  // namespace mag
