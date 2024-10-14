#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>

#include <chrono>
#include <limits>
#include <vulkan/vulkan.hpp>

#include "core/event.hpp"
#include "core/keys.hpp"
#include "core/math.hpp"
#include "core/types.hpp"

namespace mag
{
    using namespace mag::math;

    struct WindowOptions
    {
            static constexpr uvec2 MAX_SIZE = uvec2(std::numeric_limits<u32>().max());
            static constexpr ivec2 CENTER_POS = ivec2(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

            const EventCallback& event_callback;
            uvec2 size = MAX_SIZE;
            ivec2 position = CENTER_POS;
            str title = "Magnolia";
            str window_icon = "";
    };

    class Window
    {
        public:
            Window(const WindowOptions& options);
            ~Window();

            void on_update();

            vk::SurfaceKHR create_surface(const vk::Instance instance) const;

            void sleep(const u32 ms);

            b8 set_window_icon(const str& bmp_file) const;
            void set_capture_mouse(b8 capture);
            void set_title(const str& title);
            void set_resizable(const b8 resizable);
            void set_fullscreen(const b8 fullscreen);

            b8 is_key_pressed(const Key key);
            b8 is_key_down(const Key key);
            b8 is_button_down(const Button button);
            b8 is_mouse_captured() const;
            b8 is_minimized() const;
            b8 is_fullscreen() const;

            ivec2 get_mouse_position() const;
            uvec2 get_size() const;
            f64 get_time() const;  // Ms since start
            SDL_Window* get_handle() const { return handle; };
            const std::vector<const c8*>& get_instance_extensions() const { return extensions; };

        private:
            b8 is_flag_set(const u32 flag) const;

            EventCallback event_callback;

            SDL_Window* handle = {};
            u32 update_counter = {};
            b8 ignore_mouse_motion_events = {};
            std::vector<const c8*> extensions;

            std::unordered_map<Key, b8> key_state;
            std::unordered_map<Key, u32> key_update;
            std::unordered_map<Button, b8> button_state;
            std::unordered_map<Button, u32> button_update;

            const std::chrono::time_point<std::chrono::system_clock> start_time;
    };
};  // namespace mag
