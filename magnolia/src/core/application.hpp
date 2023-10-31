#pragma once

#include <memory>

#include "core/window.hpp"

namespace mag
{
    class Application
    {
        public:
            Application() = default;
            ~Application() = default;

            b8 initialize(const str& title, const u32 width = 800, const u32 height = 600);
            void shutdown();

            void run();

        private:
            std::unique_ptr<Window> window;
    };
};  // namespace mag
