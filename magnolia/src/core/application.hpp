#pragma once

#include "core/window.hpp"

#include <memory>

namespace mag
{
    class Application
    {
        public:
            Application() = default;
            ~Application() = default;

            b8 initialize(str title, u32 width = 800, u32 height = 600);
            void shutdown();

            void run();

        private:
            std::unique_ptr<Window> window;
    };
};
