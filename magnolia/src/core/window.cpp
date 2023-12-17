#include "core/window.hpp"
#include "core/logger.hpp"

namespace mag
{
    void Window::initialize(const str& title, const uvec2& size)
    {
        this->title = title;
        this->size = size;

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
        {
            ASSERT(false, "Error initializing SDL: " + str(SDL_GetError()));
        }

        // Create application window
        if ((handle = SDL_CreateWindow(title.c_str(),
                                     SDL_WINDOWPOS_CENTERED, 
                                     SDL_WINDOWPOS_CENTERED,
                                     static_cast<i32>(size.x),
                                     static_cast<i32>(size.y),
                                     0)) == nullptr)
        {
            ASSERT(false, "Error creating window: " + str(SDL_GetError()));
        }

        // Window hints
        SDL_SetWindowResizable(handle, SDL_TRUE);
    }

    void Window::shutdown()
    {
        // Deinit SDL
        SDL_DestroyWindow(handle);
        SDL_Quit();
    }

    b8 Window::update()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    return false;
                    break;

                case SDL_WINDOWEVENT:
                    if (event.type == SDL_WINDOW_RESIZABLE)
                        SDL_GetWindowSize(handle, reinterpret_cast<i32*>(&size.x), reinterpret_cast<i32*>(&size.y));
                    break;

                default:
                    break;
            }
        }

        return true;
    }
};  // namespace mag
