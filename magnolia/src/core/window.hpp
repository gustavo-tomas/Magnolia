#pragma once

#include <SDL.h>

#include "core/types.hpp"
#include "core/math.hpp"

namespace mag
{
    class Window
    {
        public:
            void initialize(const str& title, const uvec2& size);
            void shutdown();

            b8 update();

            const uvec2& get_size() const { return size; }

        private:
            SDL_Window* handle = {};
            str title = {};
            uvec2 size = {};
    };
};  // namespace mag
