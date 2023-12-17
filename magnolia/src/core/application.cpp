#include "core/application.hpp"
#include "core/logger.hpp"

namespace mag
{
    void Application::initialize(const str& title, const u32 width, const u32 height)
    {
        WindowOptions window_options;
        window_options.size = {width, height};
        window_options.title = title;

        // Create the window
        window.initialize(window_options);
        LOG_SUCCESS("Window initialized");

        // Set window callbacks
        window.on_resize([](const uvec2& size) mutable { LOG_INFO("WINDOW RESIZE: {0}", to_str(size)); });
        window.on_key_press([](const SDL_Keycode key) mutable { LOG_INFO("KEY PRESS: {0}", SDL_GetKeyName(key)); });
        window.on_key_release([](const SDL_Keycode key) mutable { LOG_INFO("KEY RELEASE: {0}", SDL_GetKeyName(key)); });
        window.on_mouse_move([](const ivec2& mouse_pos) mutable { LOG_INFO("MOUSE MOVE: {0}", to_str(mouse_pos)); });
    }

    void Application::shutdown()
    {
        window.shutdown();
        LOG_SUCCESS("Window destroyed");
    }

    void Application::run()
    {
        while (window.update())
        {
            if (window.is_key_pressed(SDLK_ESCAPE))
                window.set_capture_mouse(!window.is_mouse_captured());
        }
    }
};  // namespace mag
