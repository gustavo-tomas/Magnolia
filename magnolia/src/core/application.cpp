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

        // Create the rendeerer
        renderer.initialize(window);
        LOG_SUCCESS("Renderer initialized");

        // Set window callbacks
        window.on_resize(
            [&](const uvec2& size) mutable
            {
                LOG_INFO("WINDOW RESIZE: {0}", to_str(size));
                renderer.on_resize(size);
            });

        window.on_key_press([](const SDL_Keycode key) mutable { LOG_INFO("KEY PRESS: {0}", SDL_GetKeyName(key)); });
        window.on_key_release([](const SDL_Keycode key) mutable { LOG_INFO("KEY RELEASE: {0}", SDL_GetKeyName(key)); });
        window.on_mouse_move([](const ivec2& mouse_pos) mutable { LOG_INFO("MOUSE MOVE: {0}", to_str(mouse_pos)); });
    }

    void Application::shutdown()
    {
        renderer.shutdown();
        LOG_SUCCESS("Renderer destroyed");

        window.shutdown();
        LOG_SUCCESS("Window destroyed");
    }

    void Application::run()
    {
        u64 last_time = 0, curr_time = SDL_GetPerformanceCounter(), frame_counter = 0;
        f64 time_counter = 0.0, dt = 0.0;

        while (window.update())
        {
            // Calculate dt
            last_time = curr_time;
            curr_time = SDL_GetPerformanceCounter();
            dt = (curr_time - last_time) * 1000.0 / (SDL_GetPerformanceFrequency() * 1000.0);

            frame_counter++;
            time_counter += dt;
            if (time_counter >= 1.0)
            {
                LOG_INFO("CPU: {0:.3f} ms/frame - {1} fps", 1000.0 / static_cast<f64>(frame_counter), frame_counter);
                frame_counter = time_counter = 0;
            }

            if (window.is_key_pressed(SDLK_ESCAPE)) window.set_capture_mouse(!window.is_mouse_captured());
            renderer.update();
        }
    }
};  // namespace mag
