#include "core/window.hpp"

#include <thread>

#include "core/event.hpp"
#include "core/logger.hpp"

namespace mag
{
    Window::Window(const WindowOptions& options)
        : event_callback(options.event_callback), start_time(std::chrono::system_clock::now())
    {
        ASSERT(SDL_Init(SDL_INIT_VIDEO) == 0, "Failed to initialize SDL: " + str(SDL_GetError()));

        i32 width = 800, height = 600;

        if (options.size == WindowOptions::MAX_SIZE)
        {
            SDL_DisplayMode display_mode;
            if (SDL_GetDesktopDisplayMode(0, &display_mode) != 0)
            {
                LOG_ERROR("Failed to retrieve display mode: {0}", SDL_GetError());
            }

            else
            {
                width = display_mode.w;
                height = display_mode.h;
            }
        }

        const u32 flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

        handle = SDL_CreateWindow(options.title.c_str(), options.position.x, options.position.y, width, height, flags);

        ASSERT(handle != nullptr, "Failed to create SDL window: " + str(SDL_GetError()));

        u32 count = 0;
        ASSERT(SDL_Vulkan_GetInstanceExtensions(this->handle, &count, nullptr),
               "Failed to enumerate window extensions: " + str(SDL_GetError()));

        extensions.resize(count);
        ASSERT(SDL_Vulkan_GetInstanceExtensions(this->handle, &count, extensions.data()),
               "Failed to get extensions: " + str(SDL_GetError()));

        // Set application icon
        const str file = "magnolia/assets/images/application_icon.bmp";

        SDL_Surface* icon = SDL_LoadBMP(file.c_str());

        if (!icon)
        {
            LOG_ERROR("Failed to load application icon: {0}", str(SDL_GetError()));
        }

        else
        {
            SDL_SetWindowIcon(handle, icon);
            SDL_FreeSurface(icon);
        }
    }

    Window::~Window()
    {
        SDL_DestroyWindow(handle);
        SDL_Quit();
    }

    void Window::update()
    {
        update_counter++;

        SDL_Event e;

        while (SDL_PollEvent(&e) != 0)
        {
            const Key key = static_cast<Key>(e.key.keysym.sym);
            const Button button = static_cast<Button>(e.button.button);

            switch (e.type)
            {
                case SDL_KEYDOWN:
                {
                    auto event = KeyPressEvent(key);
                    event_callback(event);

                    if (e.key.repeat == 1) continue;
                    key_state[key] = true;
                    key_update[key] = update_counter;
                }
                break;

                case SDL_KEYUP:
                {
                    auto event = KeyReleaseEvent(key);
                    event_callback(event);

                    key_state[key] = false;
                    key_update[key] = update_counter;
                }
                break;

                case SDL_MOUSEMOTION:
                {
                    // Ignore first mouse move after capturing cursor
                    if (!ignore_mouse_motion_events)
                    {
                        auto event = MouseMoveEvent(e.motion.xrel, e.motion.yrel);
                        event_callback(event);
                    }

                    ignore_mouse_motion_events = false;
                }
                break;

                case SDL_MOUSEWHEEL:
                {
                    auto event = MouseScrollEvent(e.wheel.x, e.wheel.y);
                    event_callback(event);
                }
                break;

                case SDL_MOUSEBUTTONDOWN:
                {
                    auto event = MousePressEvent(button);
                    event_callback(event);

                    button_state[button] = true;
                    button_update[button] = update_counter;
                }
                break;

                case SDL_MOUSEBUTTONUP:
                    button_state[button] = false;
                    button_update[button] = update_counter;
                    break;

                case SDL_WINDOWEVENT:
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        auto event = WindowResizeEvent(e.window.data1, e.window.data2);
                        event_callback(event);
                    }

                    else if (e.window.event == SDL_WINDOWEVENT_CLOSE)
                    {
                        auto event = WindowCloseEvent();
                        event_callback(event);
                    }
                    break;
            }

            auto event = NativeEvent(&e);
            event_callback(event);
        }
    }

    vk::SurfaceKHR Window::create_surface(const vk::Instance instance) const
    {
        VkSurfaceKHR surface = 0;
        ASSERT(SDL_Vulkan_CreateSurface(handle, instance, &surface),
               "Failed to create surface: " + str(SDL_GetError()));

        return surface;
    }

    void Window::sleep(const u32 ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

    b8 Window::is_key_pressed(const Key key) { return key_state[key] && (key_update[key] == update_counter); }

    b8 Window::is_key_down(const Key key) { return key_state[key]; }

    b8 Window::is_button_down(const Button button) { return button_state[button]; }

    b8 Window::is_mouse_captured() const { return static_cast<b8>(SDL_GetRelativeMouseMode()); }

    b8 Window::is_minimized() const
    {
        const auto size = this->get_size();
        return is_flag_set(SDL_WINDOW_MINIMIZED) || (size.x < 1 || size.y < 1);

        // Might be worth checking these too: || !(flags & SDL_WINDOW_INPUT_FOCUS) || !(flags & SDL_WINDOW_MOUSE_FOCUS)
    }

    b8 Window::is_fullscreen() const { return is_flag_set(SDL_WINDOW_FULLSCREEN_DESKTOP); }

    b8 Window::is_flag_set(const u32 flag) const
    {
        const u32 flags = SDL_GetWindowFlags(this->handle);
        return (flag & flags);
    }

    void Window::set_capture_mouse(b8 capture)
    {
        // Oh SDL...
        if (SDL_SetRelativeMouseMode(static_cast<SDL_bool>(capture)) != 0)
        {
            LOG_ERROR("Failed to set mouse mode: {0}", SDL_GetError());
        }

        ignore_mouse_motion_events = true;
    }

    void Window::set_title(const str& title) { SDL_SetWindowTitle(handle, title.c_str()); }

    void Window::set_resizable(const b8 resizable)
    {
        SDL_SetWindowResizable(this->handle, static_cast<SDL_bool>(resizable));
    }

    void Window::set_fullscreen(const b8 fullscreen)
    {
        const SDL_WindowFlags flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
        if (SDL_SetWindowFullscreen(this->handle, fullscreen ? flag : 0) != 0)
        {
            LOG_ERROR("Failed to set fullscreen mode: {0}", SDL_GetError());
        }
    }

    ivec2 Window::get_mouse_position() const
    {
        ivec2 mouse_pos;
        SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        return mouse_pos;
    }

    uvec2 Window::get_size() const
    {
        uvec2 size;
        SDL_Vulkan_GetDrawableSize(handle, reinterpret_cast<i32*>(&size.x), reinterpret_cast<i32*>(&size.y));
        return size;
    }

    f64 Window::get_time() const
    {
        // Ms since start
        auto current_time = std::chrono::system_clock::now();
        std::chrono::duration<f64> elapsed_seconds = current_time - start_time;

        return elapsed_seconds.count();
    }
};  // namespace mag
