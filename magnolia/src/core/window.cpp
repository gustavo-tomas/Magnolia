#include "core/window.hpp"

#include <chrono>
#include <vulkan/vulkan.hpp>

#include "SDL.h"
#include "SDL_vulkan.h"
#include "core/application.hpp"
#include "core/assert.hpp"
#include "core/buffer.hpp"
#include "core/event.hpp"
#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "private/key_mappings.hpp"

namespace mag
{
    namespace window
    {
        struct State
        {
                EventCallback event_callback;

                SDL_Window* handle = nullptr;
                u32 update_counter = 0;
                b8 ignore_mouse_motion_events = false;
                std::vector<const c8*> extensions;

                std::unordered_map<SDL_Keycode, b8> key_state;
                std::unordered_map<SDL_Keycode, u32> key_update;
                std::unordered_map<i32, b8> button_state;
                std::unordered_map<i32, u32> button_update;

                const std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
        };

        static State* state = nullptr;

        b8 initialize(const WindowOptions& options)
        {
            state = new State;
            state->event_callback = options.event_callback;

            MAG_ASSERT(SDL_Init(SDL_INIT_VIDEO) == 0, "Failed to initialize SDL: " + str(SDL_GetError()));

            i32 width = 800, height = 600;

            // Determines window size automatically
            if (options.size.x == WindowOptions::MaxSize.x || options.size.y == WindowOptions::MaxSize.y)
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

            // User provided window size
            else
            {
                width = options.size.x;
                height = options.size.y;
            }

            const u32 flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

            state->handle = SDL_CreateWindow(
                options.title.c_str(), (options.position.x == Max_I32) ? SDL_WINDOWPOS_CENTERED : options.position.x,
                (options.position.y == Max_I32) ? SDL_WINDOWPOS_CENTERED : options.position.y, width, height, flags);

            MAG_ASSERT(state->handle != nullptr, "Failed to create SDL window: " + str(SDL_GetError()));

            u32 count = 0;
            MAG_ASSERT(SDL_Vulkan_GetInstanceExtensions(state->handle, &count, nullptr),
                       "Failed to enumerate window extensions: " + str(SDL_GetError()));

            state->extensions.resize(count);
            MAG_ASSERT(SDL_Vulkan_GetInstanceExtensions(state->handle, &count, state->extensions.data()),
                       "Failed to get extensions: " + str(SDL_GetError()));

            if (!options.window_icon.empty())
            {
                set_window_icon(options.window_icon);
            }

            return state != nullptr;
        }

        void shutdown()
        {
            SDL_DestroyWindow(state->handle);
            SDL_Quit();

            delete state;
        }

        void on_update()
        {
            state->update_counter++;

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
                        state->event_callback(event);

                        if (e.key.repeat == 1)
                        {
                            continue;
                        }

                        state->key_state[key] = true;
                        state->key_update[key] = state->update_counter;
                    }
                    break;

                    case SDL_KEYUP:
                    {
                        auto event = KeyReleaseEvent(KeycodeMapper::from_SDL_keycode(key));
                        state->event_callback(event);

                        state->key_state[key] = false;
                        state->key_update[key] = state->update_counter;
                    }
                    break;

                    case SDL_MOUSEMOTION:
                    {
                        // Ignore first mouse move after capturing cursor
                        if (!state->ignore_mouse_motion_events)
                        {
                            auto event = MouseMoveEvent(e.motion.xrel, e.motion.yrel);
                            state->event_callback(event);
                        }

                        state->ignore_mouse_motion_events = false;
                    }
                    break;

                    case SDL_MOUSEWHEEL:
                    {
                        auto event = MouseScrollEvent(e.wheel.x, e.wheel.y);
                        state->event_callback(event);
                    }
                    break;

                    case SDL_MOUSEBUTTONDOWN:
                    {
                        auto event = MousePressEvent(KeycodeMapper::from_SDL_button(button));
                        state->event_callback(event);

                        state->button_state[button] = true;
                        state->button_update[button] = state->update_counter;
                    }
                    break;

                    case SDL_MOUSEBUTTONUP:
                        state->button_state[button] = false;
                        state->button_update[button] = state->update_counter;
                        break;

                    case SDL_WINDOWEVENT:
                        if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                        {
                            auto event = WindowResizeEvent(e.window.data1, e.window.data2);
                            state->event_callback(event);
                        }

                        else if (e.window.event == SDL_WINDOWEVENT_CLOSE)
                        {
                            auto event = WindowCloseEvent();
                            state->event_callback(event);
                        }
                        break;
                }

                auto event = NativeEvent(&e);
                state->event_callback(event);
            }
        }

        void create_surface(const void* instance, void* surface)
        {
            vk::Instance vk_instance = *reinterpret_cast<const vk::Instance*>(instance);
            MAG_ASSERT(SDL_Vulkan_CreateSurface(state->handle, vk_instance, reinterpret_cast<VkSurfaceKHR*>(surface)),
                       "Failed to create surface: " + str(SDL_GetError()));
        }

        b8 is_key_pressed(const Key key)
        {
            return state->key_state[KeycodeMapper::to_SDL_keycode(key)] &&
                   (state->key_update[KeycodeMapper::to_SDL_keycode(key)] == state->update_counter);
        }

        b8 is_button_pressed(const Button button)
        {
            return state->button_state[KeycodeMapper::to_SDL_button(button)] &&
                   (state->button_update[KeycodeMapper::to_SDL_button(button)] == state->update_counter);
        }

        b8 is_key_down(const Key key) { return state->key_state[KeycodeMapper::to_SDL_keycode(key)]; }

        b8 is_button_down(const Button button) { return state->button_state[KeycodeMapper::to_SDL_button(button)]; }

        b8 is_mouse_captured() { return static_cast<b8>(SDL_GetRelativeMouseMode()); }

        b8 is_flag_set(const u32 flag)
        {
            const u32 flags = SDL_GetWindowFlags(state->handle);
            return (flag & flags);
        }

        b8 is_minimized()
        {
            const auto size = get_size();
            return is_flag_set(SDL_WINDOW_MINIMIZED) || (size.x < 1 || size.y < 1);

            // Might be worth checking these too: || !(flags & SDL_WINDOW_INPUT_FOCUS) || !(flags &
            // SDL_WINDOW_MOUSE_FOCUS)
        }

        b8 is_fullscreen() { return is_flag_set(SDL_WINDOW_FULLSCREEN_DESKTOP); }

        b8 set_window_icon(const str& bmp_file)
        {
            Buffer buffer;
            if (!fs::read_binary_data(bmp_file, buffer))
            {
                LOG_ERROR("Failed to read file: '{0}'", bmp_file);
                return false;
            }

            SDL_RWops* rw = SDL_RWFromMem(buffer.cast<void*>(), buffer.get_size());

            if (!rw)
            {
                LOG_ERROR("Failed to read from memory: '{0}'", SDL_GetError());
                return false;
            }

            SDL_Surface* icon = SDL_LoadBMP_RW(rw, 1);

            if (!icon)
            {
                LOG_ERROR("Failed to load application icon: '{0}'", SDL_GetError());
                return false;
            }

            SDL_SetWindowIcon(state->handle, icon);
            SDL_FreeSurface(icon);

            return true;
        }

        void set_capture_mouse(b8 capture)
        {
            // Oh SDL...
            if (SDL_SetRelativeMouseMode(static_cast<SDL_bool>(capture)) != 0)
            {
                LOG_ERROR("Failed to set mouse mode: {0}", SDL_GetError());
            }

            state->ignore_mouse_motion_events = true;
        }

        void set_title(const str& title) { SDL_SetWindowTitle(state->handle, title.c_str()); }

        void set_resizable(const b8 resizable)
        {
            SDL_SetWindowResizable(state->handle, static_cast<SDL_bool>(resizable));
        }

        void set_fullscreen(const b8 fullscreen)
        {
            const SDL_WindowFlags flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
            if (SDL_SetWindowFullscreen(state->handle, fullscreen ? flag : 0) != 0)
            {
                LOG_ERROR("Failed to set fullscreen mode: {0}", SDL_GetError());
            }
        }

        math::ivec2 get_mouse_position()
        {
            math::ivec2 mouse_pos;
            SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
            return mouse_pos;
        }

        math::uvec2 get_size()
        {
            math::uvec2 size;
            SDL_Vulkan_GetDrawableSize(state->handle, reinterpret_cast<i32*>(&size.x), reinterpret_cast<i32*>(&size.y));
            return size;
        }

        f64 get_time()
        {
            // Ms since start
            auto current_time = std::chrono::system_clock::now();
            std::chrono::duration<f64> elapsed_seconds = current_time - state->start_time;

            return elapsed_seconds.count() * 1000.0;
        }

        void* get_handle() { return state->handle; }

        const std::vector<const c8*>& get_instance_extensions() { return state->extensions; }
    };  // namespace window
};      // namespace mag
