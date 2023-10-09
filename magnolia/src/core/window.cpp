#include "core/window.hpp"
#include "core/logger.hpp"

namespace mag
{
    bool Window::initialize(str title, u32 width, u32 height)
    {
        this->title = title;
        this->width = width;
        this->height = height;

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
        {
            const str error = SDL_GetError();
            LOG_ERROR("Error initializing SDL: {0}", error);
            
            return false;
        }

        // Create application window
        if ((window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            width,
            height,
            0)) == nullptr)
        {
            const str error = SDL_GetError();
            LOG_ERROR("Error creating window: {0}", error);
            
            return false;
        }

        return true;
    }

    void Window::shutdown()
    {
        // Deinit SDL
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    u32 Window::get_width()
    {
        return width;
    }

    u32 Window::get_height()
    {
        return height;
    }
};
