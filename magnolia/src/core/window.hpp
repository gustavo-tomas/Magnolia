#pragma once

#include "core/types.hpp"

#include <SDL2/SDL.h>

namespace mag
{
    class Window
    {
        public:
            Window() = default;
            ~Window() = default;

            b8 initialize(str title, u32 width, u32 height);
            void shutdown();

            u32 get_width();
            u32 get_height();

        private:
            SDL_Window* window;

            str title;
            u32 width;
            u32 height;        
    };
};
