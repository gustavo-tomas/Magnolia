#pragma once

#include <SDL.h>

#include "core/types.hpp"

namespace mag
{
    class Window
    {
        public:
            void initialize(const str& title, const u32 width, const u32 height);
            void shutdown();

            b8 update();

            u32 get_width() const { return width; }
            u32 get_height() const { return height; }

        private:
            SDL_Window* handle;

            str title;
            u32 width;
            u32 height;
    };
};  // namespace mag
