#include "private/key_mappings.hpp"

namespace mag
{
    // clang-format off
    
    // Button mappings

    SDL_Keycode KeycodeMapper::to_SDL_keycode(Key key)
    {
        (void)key;
        return 0;
    }

    Key KeycodeMapper::from_SDL_keycode(SDL_Keycode sdl_keycode)
    {
        (void)sdl_keycode;
        return Key::Unknown;
    }

    i32 KeycodeMapper::to_SDL_button(const Button button)
    {
        switch (button)
        {
            case Button::Left:    return SDL_BUTTON_LEFT;
            case Button::Middle:  return SDL_BUTTON_MIDDLE;
            case Button::Right:   return SDL_BUTTON_RIGHT;
            case Button::X1:      return SDL_BUTTON_X1;
            case Button::X2:      return SDL_BUTTON_X2;
            case Button::Unknown:
            default:              return 0;
        }
    }

    Button KeycodeMapper::from_SDL_button(const i32 sdl_button)
    {
        switch (sdl_button)
        {
            case SDL_BUTTON_LEFT:   return Button::Left;
            case SDL_BUTTON_MIDDLE: return Button::Middle;
            case SDL_BUTTON_RIGHT:  return Button::Right;
            case SDL_BUTTON_X1:     return Button::X1;
            case SDL_BUTTON_X2:     return Button::X2;
            default:                return Button::Unknown;
        }
    }

    // clang-format on
};  // namespace mag
