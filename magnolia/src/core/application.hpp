#pragma once

#include "core/window.hpp"
#include "renderer/renderer.hpp"

namespace mag
{
    class Application
    {
        public:
            void initialize(const str& title, const u32 width = 800, const u32 height = 600);
            void shutdown();

            void run();

        private:
            Window window;
            Renderer renderer;
    };
};  // namespace mag
