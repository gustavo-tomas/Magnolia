#include "core/window.hpp"

#include "core/logger.hpp"

namespace mag
{
    b8 Window::initialize(const str& title, const u32 width, const u32 height)
    {
        this->title = title;
        this->width = width;
        this->height = height;
        quit_requested = false;

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
        {
            const str error = SDL_GetError();
            LOG_ERROR("Error initializing SDL: {0}", error);

            return false;
        }

        // Create application window
        if ((window = SDL_CreateWindow(
                 title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0)) == nullptr)
        {
            const str error = SDL_GetError();
            LOG_ERROR("Error creating window: {0}", error);

            return false;
        }

        // Window hints
        SDL_SetWindowResizable(window, SDL_TRUE);

        return true;
    }

    void Window::shutdown()
    {
        // Deinit SDL
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Window::update()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    quit_requested = true;
                    break;

                case SDL_WINDOWEVENT:
                    if (event.type == SDL_WINDOW_RESIZABLE)
                        SDL_GetWindowSize(window, reinterpret_cast<i32*>(&width), reinterpret_cast<i32*>(&height));
                    break;

                default:
                    break;
            }
        }
    }

    b8 Window::quit()
    {
        return quit_requested;
    }

    u32 Window::get_width()
    {
        return width;
    }

    u32 Window::get_height()
    {
        return height;
    }
};  // namespace mag
