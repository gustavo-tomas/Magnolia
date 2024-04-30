#include "core/window.hpp"

#include "core/event.hpp"
#include "core/logger.hpp"

namespace mag
{
    Window::Window(const WindowOptions& options) : event_manager(options.event_manager)
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
            const SDL_Keycode key = e.key.keysym.sym;
            const u8 button = e.button.button;

            switch (e.type)
            {
                case SDL_KEYDOWN:
                {
                    auto event = KeyPressEvent(key);
                    event_manager.emit(EventType::KeyPress, event);

                    if (e.key.repeat == 1) continue;
                    key_state[key] = true;
                    key_update[key] = update_counter;
                }
                break;

                case SDL_KEYUP:
                {
                    auto event = KeyReleaseEvent(key);
                    event_manager.emit(EventType::KeyRelease, event);

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
                        event_manager.emit(EventType::MouseMove, event);
                    }

                    ignore_mouse_motion_events = false;
                }
                break;

                case SDL_MOUSEWHEEL:
                {
                    auto event = MouseScrollEvent(e.wheel.x, e.wheel.y);
                    event_manager.emit(EventType::MouseScroll, event);
                }
                break;

                case SDL_MOUSEBUTTONDOWN:
                {
                    auto event = MousePressEvent(button);
                    event_manager.emit(EventType::MousePress, event);

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
                        event_manager.emit(EventType::WindowResize, event);
                    }

                    else if (e.window.event == SDL_WINDOWEVENT_CLOSE)
                    {
                        auto event = WindowCloseEvent();
                        event_manager.emit(EventType::WindowClose, event);
                    }
                    break;
            }

            auto event = SDLEvent(e);
            event_manager.emit(EventType::SDLEvent, event);
        }
    }

    vk::SurfaceKHR Window::create_surface(const vk::Instance instance) const
    {
        VkSurfaceKHR surface = 0;
        ASSERT(SDL_Vulkan_CreateSurface(handle, instance, &surface),
               "Failed to create surface: " + str(SDL_GetError()));

        return surface;
    }

    b8 Window::is_key_pressed(const SDL_Keycode key) { return key_state[key] && (key_update[key] == update_counter); }

    b8 Window::is_key_down(const SDL_Keycode key) { return key_state[key]; }

    b8 Window::is_button_down(const u8 button) { return button_state[button]; }

    b8 Window::is_mouse_captured() const { return static_cast<b8>(SDL_GetRelativeMouseMode()); }

    b8 Window::is_minimized() const
    {
        const auto size = this->get_size();
        return is_flag_set(SDL_WINDOW_MINIMIZED) || (size.x < 1 || size.y < 1);

        // Might be worth checking these too: || !(flags & SDL_WINDOW_INPUT_FOCUS) || !(flags & SDL_WINDOW_MOUSE_FOCUS)
    }

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

    void Window::set_fullscreen(const u32 flags)
    {
        if (SDL_SetWindowFullscreen(this->handle, flags) != 0)
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
};  // namespace mag
