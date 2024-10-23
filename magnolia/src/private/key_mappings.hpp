#pragma once

#include "SDL_keycode.h"
#include "core/keys.hpp"

namespace mag
{
    class KeycodeMapper
    {
        public:
            static SDL_Keycode to_SDL_keycode(Key key);
            static Key from_SDL_keycode(SDL_Keycode sdl_keycode);

            static i32 to_SDL_button(const Button button);
            static Button from_SDL_button(const i32 sdl_button);
    };
};  // namespace mag
