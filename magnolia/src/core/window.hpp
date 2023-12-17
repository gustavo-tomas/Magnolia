#pragma once

#include <SDL.h>

#include "core/types.hpp"

namespace mag
{
    class Window
    {
        public:
            Window() = default;
            ~Window() = default;

            b8 initialize(const str& title, const u32 width, const u32 height);
            void shutdown();

            void update();

            b8 quit();

            u32 get_width();
            u32 get_height();

        private:
            SDL_Window* window;

            str title;
            u32 width;
            u32 height;
            b8 quit_requested;
    };
};  // namespace mag
