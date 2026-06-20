#include "magnolia/platform/window.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <algorithm>

#include "conversions.hpp"
#include "magnolia/core/assert.hpp"
#include "magnolia/core/buffer.hpp"
#include "magnolia/core/event.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"
#include "magnolia/platform/platform.hpp"
#include "magnolia/threads/thread.hpp"
#include "magnolia/tools/console.hpp"

namespace mag
{
    namespace window
    {
        struct State
        {
                EventCallback event_callback = [](const Event&) {};

                SDL_Window* handle = nullptr;
                u32 update_counter = 0;
                b8 mouse_moved = false;
                b8 window_resized = false;
                SDL_Event last_window_resize_event = {};
                SDL_Event last_mouse_move_event = {};
                std::vector<const c8*> extensions;

                f64 dt = 0;
                i32 target_frame_rate = -1;

                std::unordered_map<SDL_Keycode, b8> key_state;
                std::unordered_map<SDL_Keycode, u32> key_update;
                std::unordered_map<i32, b8> button_state;
                std::unordered_map<i32, u32> button_update;
        };

        static State* state = nullptr;

        b8 initialize(const WindowOptions& options)
        {
            state = new State();

            MAG_ASSERT(SDL_Init(SDL_INIT_VIDEO), "Failed to initialize SDL: '{}'", SDL_GetError());

            SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;

            const i32 width = static_cast<i32>(options.size.x);
            const i32 height = static_cast<i32>(options.size.y);
            i32 position_x = options.position.x;
            i32 position_y = options.position.y;

            if (options.size.x == WindowOptions::MaxSize || options.size.y == WindowOptions::MaxSize)
            {
                flags |= SDL_WINDOW_MAXIMIZED;
            }

            if (options.position.x == WindowOptions::CenterPos)
            {
                position_x = SDL_WINDOWPOS_CENTERED;
            }

            if (options.position.y == WindowOptions::CenterPos)
            {
                position_y = SDL_WINDOWPOS_CENTERED;
            }

            state->handle = SDL_CreateWindow(options.title.c_str(), width, height, flags);

            MAG_ASSERT(state->handle != nullptr, "Failed to create SDL window: '{}'", SDL_GetError());

            SDL_SetWindowPosition(state->handle, position_x, position_y);

            u32 count = 0;
            const c8* const* extensions = SDL_Vulkan_GetInstanceExtensions(&count);

            MAG_ASSERT(count, "Failed to get window extensions: '{}'", SDL_GetError());

            state->extensions.resize(count);
            std::copy(extensions, extensions + count, state->extensions.data());

            if (!options.window_icon.empty())
            {
                set_window_icon(options.window_icon);
            }

            set_target_frame_rate(options.target_frame_rate);

            SDL_ShowWindow(state->handle);

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

            static f64 curr_time = 0;
            static f64 last_time = plat::get_time();

            // Delay if needed
            const f64 target_frame_rate = static_cast<f64>(state->target_frame_rate);
            if (target_frame_rate > 0.0)
            {
                const f64 delay = (1000.0 / target_frame_rate) - (plat::get_time() - last_time);
                if (delay > 0.0)
                {
                    thread::sleep(delay);
                }
            }

            // Calculate dt
            curr_time = plat::get_time();
            state->dt = (curr_time - last_time) / 1000.0;  // convert from ms to seconds
            last_time = curr_time;

            SDL_Event e;

            while (SDL_PollEvent(&e))
            {
                const SDL_WindowID window_id = e.window.windowID;

                if (window_id == console::get_window_id())
                {
                    console::on_event(&e);
                    continue;
                }

                const SDL_Keycode key = e.key.key;
                const u8 button = e.button.button;

                switch (e.type)
                {
                    case SDL_EVENT_KEY_DOWN:
                    {
                        auto event = KeyPressEvent(sdl_to_mag_key(key));
                        state->event_callback(event);

                        if (e.key.repeat)
                        {
                            continue;
                        }

                        state->key_state[key] = true;
                        state->key_update[key] = state->update_counter;
                    }
                    break;

                    case SDL_EVENT_KEY_UP:
                    {
                        auto event = KeyReleaseEvent(sdl_to_mag_key(key));
                        state->event_callback(event);

                        state->key_state[key] = false;
                        state->key_update[key] = state->update_counter;
                    }
                    break;

                    case SDL_EVENT_MOUSE_MOTION:
                    {
                        state->last_mouse_move_event = e;
                        state->mouse_moved = true;
                    }
                    break;

                    case SDL_EVENT_MOUSE_WHEEL:
                    {
                        auto event = MouseScrollEvent(e.wheel.x, e.wheel.y);
                        state->event_callback(event);
                    }
                    break;

                    case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    {
                        auto event = MousePressEvent(sdl_to_mag_button(button));
                        state->event_callback(event);

                        state->button_state[button] = true;
                        state->button_update[button] = state->update_counter;
                    }
                    break;

                    case SDL_EVENT_MOUSE_BUTTON_UP:
                    {
                        state->button_state[button] = false;
                        state->button_update[button] = state->update_counter;
                    }
                    break;

                    case SDL_EVENT_WINDOW_RESIZED:
                    {
                        state->last_window_resize_event = e;
                        state->window_resized = true;
                    }
                    break;

                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    {
                        auto event = WindowCloseEvent();
                        state->event_callback(event);
                    }
                    break;

                    default:
                        break;
                }
            }

            // Emit window resize event (the last one) only once per frame
            if (state->window_resized)
            {
                auto event = WindowResizeEvent(state->last_window_resize_event.window.data1,
                                               state->last_window_resize_event.window.data2);

                state->event_callback(event);
                state->window_resized = false;
            }

            // The mouse motion event is emitted many times in a single frame and lags a lot. To fix this we
            // restrict this event to happen only once per frame
            if (state->mouse_moved)
            {
                const i32 x_direction = static_cast<i32>(state->last_mouse_move_event.motion.xrel);
                const i32 y_direction = static_cast<i32>(state->last_mouse_move_event.motion.yrel);
                const i32 x = static_cast<i32>(state->last_mouse_move_event.motion.x);
                const i32 y = static_cast<i32>(state->last_mouse_move_event.motion.y);

                auto event = MouseMoveEvent(x_direction, y_direction, x, y);

                state->event_callback(event);
                state->mouse_moved = false;
            }

            console::on_update();
        }

        void create_surface(const void* instance, void* surface)
        {
            MAG_ASSERT(SDL_Vulkan_CreateSurface(state->handle, *reinterpret_cast<const VkInstance*>(instance), nullptr,
                                                reinterpret_cast<VkSurfaceKHR*>(surface)),
                       "Failed to create surface: '{}'", SDL_GetError());
        }

        b8 is_key_pressed(const Key key)
        {
            return state->key_state[mag_to_sdl(key)] && (state->key_update[mag_to_sdl(key)] == state->update_counter);
        }

        b8 is_button_pressed(const Button button)
        {
            return state->button_state[mag_to_sdl(button)] &&
                   (state->button_update[mag_to_sdl(button)] == state->update_counter);
        }

        b8 is_key_down(const Key key) { return state->key_state[mag_to_sdl(key)]; }

        b8 is_button_down(const Button button) { return state->button_state[mag_to_sdl(button)]; }

        b8 is_mouse_captured() { return SDL_GetWindowRelativeMouseMode(state->handle); }

        static b8 is_flag_set(const SDL_WindowFlags flag)
        {
            const SDL_WindowFlags flags = SDL_GetWindowFlags(state->handle);
            return (flag & flags) != 0;
        }

        b8 is_minimized()
        {
            const auto size = get_size();
            return is_flag_set(SDL_WINDOW_MINIMIZED) || (size.x < 1 || size.y < 1);

            // Might be worth checking these too: || !(flags & SDL_WINDOW_INPUT_FOCUS) || !(flags &
            // SDL_WINDOW_MOUSE_FOCUS)
        }

        b8 is_fullscreen() { return is_flag_set(SDL_WINDOW_FULLSCREEN); }

        b8 set_window_icon(const str& bmp_file)
        {
            Buffer buffer;
            if (!fs::read_binary_data(bmp_file, buffer))
            {
                LOG_ERROR("Failed to read file: '{0}'", bmp_file);
                return false;
            }

            SDL_IOStream* rw = SDL_IOFromMem(buffer.cast<void>(), buffer.get_size());

            if (rw == nullptr)
            {
                LOG_ERROR("Failed to read from memory: '{0}'", SDL_GetError());
                return false;
            }

            SDL_Surface* icon = SDL_LoadBMP_IO(rw, true);

            if (icon == nullptr)
            {
                LOG_ERROR("Failed to load application icon: '{0}'", SDL_GetError());
                return false;
            }

            b8 result = true;

            if (!SDL_SetWindowIcon(state->handle, icon))
            {
                LOG_ERROR("Failed to set application icon: '{0}'", SDL_GetError());
                result = false;
            }

            SDL_DestroySurface(icon);

            return result;
        }

        void set_capture_mouse(const b8 capture)
        {
            if (!SDL_SetWindowRelativeMouseMode(state->handle, capture))
            {
                LOG_ERROR("Failed to set mouse mode: {0}", SDL_GetError());
                return;
            }
        }

        void set_mouse_position(const i32 x, const i32 y)
        {
            SDL_WarpMouseInWindow(state->handle, static_cast<f32>(x), static_cast<f32>(y));
        }

        void set_title(const str& title) { SDL_SetWindowTitle(state->handle, title.c_str()); }

        void set_resizable(const b8 resizable) { SDL_SetWindowResizable(state->handle, resizable); }

        void set_fullscreen(const b8 fullscreen)
        {
            if (SDL_SetWindowFullscreen(state->handle, fullscreen))
            {
                LOG_ERROR("Failed to set fullscreen mode: {0}", SDL_GetError());
            }
        }

        void set_event_callback(const EventCallback& callback) { state->event_callback = callback; }

        void set_target_frame_rate(const i32 frame_rate) { state->target_frame_rate = frame_rate; }

        f64 get_delta_time() { return state->dt; }

        math::ivec2 get_mouse_position()
        {
            math::vec2 mouse_pos;
            SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
            return math::ivec2(mouse_pos);
        }

        math::uvec2 get_window_center()
        {
            const math::uvec2 window_size = get_size();
            const math::uvec2 window_center = window_size / 2U;

            return window_center;
        }

        math::uvec2 get_size()
        {
            math::uvec2 size;
            SDL_GetWindowSizeInPixels(state->handle, reinterpret_cast<i32*>(&size.x), reinterpret_cast<i32*>(&size.y));
            return size;
        }

        const std::vector<const c8*>& get_instance_extensions() { return state->extensions; }
    };  // namespace window
};  // namespace mag
