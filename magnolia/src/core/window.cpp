#include "core/window.hpp"

#include <chrono>
#include <thread>
#include <vulkan/vulkan.hpp>

#include "SDL.h"
#include "SDL_vulkan.h"
#include "core/assert.hpp"
#include "core/event.hpp"
#include "core/logger.hpp"
#include "private/key_mappings.hpp"

namespace mag
{
    struct Window::IMPL
    {
            explicit IMPL(const WindowOptions& options)
                : event_callback(options.event_callback), start_time(std::chrono::system_clock::now())
            {
            }

            EventCallback event_callback;

            SDL_Window* handle = {};
            u32 update_counter = {};
            b8 ignore_mouse_motion_events = {};
            std::vector<const c8*> extensions;

            std::unordered_map<SDL_Keycode, b8> key_state;
            std::unordered_map<SDL_Keycode, u32> key_update;
            std::unordered_map<i32, b8> button_state;
            std::unordered_map<i32, u32> button_update;

            const std::chrono::time_point<std::chrono::system_clock> start_time;
    };

    Window::Window(const WindowOptions& options) : impl(new IMPL(options))
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

        impl->handle = SDL_CreateWindow(
            options.title.c_str(), (options.position.x == MAX_I32) ? SDL_WINDOWPOS_CENTERED : options.position.x,
            (options.position.y == MAX_I32) ? SDL_WINDOWPOS_CENTERED : options.position.y, width, height, flags);

        ASSERT(impl->handle != nullptr, "Failed to create SDL window: " + str(SDL_GetError()));

        u32 count = 0;
        ASSERT(SDL_Vulkan_GetInstanceExtensions(impl->handle, &count, nullptr),
               "Failed to enumerate window extensions: " + str(SDL_GetError()));

        impl->extensions.resize(count);
        ASSERT(SDL_Vulkan_GetInstanceExtensions(impl->handle, &count, impl->extensions.data()),
               "Failed to get extensions: " + str(SDL_GetError()));

        if (!options.window_icon.empty())
        {
            set_window_icon(options.window_icon);
        }
    }

    Window::~Window()
    {
        SDL_DestroyWindow(impl->handle);
        SDL_Quit();
    }

    void Window::on_update()
    {
        impl->update_counter++;

        SDL_Event e;

        while (SDL_PollEvent(&e) != 0)
        {
            const SDL_Keycode key = e.key.keysym.sym;
            const u8 button = e.button.button;

            switch (e.type)
            {
                case SDL_KEYDOWN:
                {
                    auto event = KeyPressEvent(KeycodeMapper::from_SDL_keycode(key));
                    impl->event_callback(event);

                    if (e.key.repeat == 1) continue;
                    impl->key_state[key] = true;
                    impl->key_update[key] = impl->update_counter;
                }
                break;

                case SDL_KEYUP:
                {
                    auto event = KeyReleaseEvent(KeycodeMapper::from_SDL_keycode(key));
                    impl->event_callback(event);

                    impl->key_state[key] = false;
                    impl->key_update[key] = impl->update_counter;
                }
                break;

                case SDL_MOUSEMOTION:
                {
                    // Ignore first mouse move after capturing cursor
                    if (!impl->ignore_mouse_motion_events)
                    {
                        auto event = MouseMoveEvent(e.motion.xrel, e.motion.yrel);
                        impl->event_callback(event);
                    }

                    impl->ignore_mouse_motion_events = false;
                }
                break;

                case SDL_MOUSEWHEEL:
                {
                    auto event = MouseScrollEvent(e.wheel.x, e.wheel.y);
                    impl->event_callback(event);
                }
                break;

                case SDL_MOUSEBUTTONDOWN:
                {
                    auto event = MousePressEvent(KeycodeMapper::from_SDL_button(button));
                    impl->event_callback(event);

                    impl->button_state[button] = true;
                    impl->button_update[button] = impl->update_counter;
                }
                break;

                case SDL_MOUSEBUTTONUP:
                    impl->button_state[button] = false;
                    impl->button_update[button] = impl->update_counter;
                    break;

                case SDL_WINDOWEVENT:
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        auto event = WindowResizeEvent(e.window.data1, e.window.data2);
                        impl->event_callback(event);
                    }

                    else if (e.window.event == SDL_WINDOWEVENT_CLOSE)
                    {
                        auto event = WindowCloseEvent();
                        impl->event_callback(event);
                    }
                    break;
            }

            auto event = NativeEvent(&e);
            impl->event_callback(event);
        }
    }

    void Window::create_surface(const void* instance, void* surface) const
    {
        vk::Instance vk_instance = *reinterpret_cast<const vk::Instance*>(instance);
        ASSERT(SDL_Vulkan_CreateSurface(impl->handle, vk_instance, reinterpret_cast<VkSurfaceKHR*>(surface)),
               "Failed to create surface: " + str(SDL_GetError()));
    }

    void Window::sleep(const u32 ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

    b8 Window::is_key_pressed(const Key key) const
    {
        return impl->key_state[KeycodeMapper::to_SDL_keycode(key)] &&
               (impl->key_update[KeycodeMapper::to_SDL_keycode(key)] == impl->update_counter);
    }

    b8 Window::is_button_pressed(const Button button) const
    {
        return impl->button_state[KeycodeMapper::to_SDL_button(button)] &&
               (impl->button_update[KeycodeMapper::to_SDL_button(button)] == impl->update_counter);
    }

    b8 Window::is_key_down(const Key key) const { return impl->key_state[KeycodeMapper::to_SDL_keycode(key)]; }

    b8 Window::is_button_down(const Button button) const
    {
        return impl->button_state[KeycodeMapper::to_SDL_button(button)];
    }

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
        const u32 flags = SDL_GetWindowFlags(impl->handle);
        return (flag & flags);
    }

    b8 Window::set_window_icon(const str& bmp_file) const
    {
        SDL_Surface* icon = SDL_LoadBMP(bmp_file.c_str());

        if (!icon)
        {
            LOG_ERROR("Failed to load application icon: {0}", str(SDL_GetError()));
            return false;
        }

        SDL_SetWindowIcon(impl->handle, icon);
        SDL_FreeSurface(icon);

        return true;
    }

    void Window::set_capture_mouse(b8 capture)
    {
        // Oh SDL...
        if (SDL_SetRelativeMouseMode(static_cast<SDL_bool>(capture)) != 0)
        {
            LOG_ERROR("Failed to set mouse mode: {0}", SDL_GetError());
        }

        impl->ignore_mouse_motion_events = true;
    }

    void Window::set_title(const str& title) { SDL_SetWindowTitle(impl->handle, title.c_str()); }

    void Window::set_resizable(const b8 resizable)
    {
        SDL_SetWindowResizable(impl->handle, static_cast<SDL_bool>(resizable));
    }

    void Window::set_fullscreen(const b8 fullscreen)
    {
        const SDL_WindowFlags flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
        if (SDL_SetWindowFullscreen(impl->handle, fullscreen ? flag : 0) != 0)
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
        SDL_Vulkan_GetDrawableSize(impl->handle, reinterpret_cast<i32*>(&size.x), reinterpret_cast<i32*>(&size.y));
        return size;
    }

    f64 Window::get_time() const
    {
        // Ms since start
        auto current_time = std::chrono::system_clock::now();
        std::chrono::duration<f64> elapsed_seconds = current_time - impl->start_time;

        return elapsed_seconds.count() * 1000.0;
    }

    void* Window::get_handle() const { return impl->handle; }

    const std::vector<const c8*>& Window::get_instance_extensions() const { return impl->extensions; }
};  // namespace mag
