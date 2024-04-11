#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>

#include <functional>
#include <limits>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"

namespace mag
{
    using namespace mag::math;

    struct WindowOptions
    {
            static constexpr uvec2 MAX_SIZE = uvec2(std::numeric_limits<u32>().max());

            uvec2 size = MAX_SIZE;
            ivec2 position = {SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED};
            str title = "Magnolia";
    };

    class Window
    {
        public:
            void initialize(const WindowOptions& options);
            void shutdown();
            b8 update();

            vk::SurfaceKHR create_surface(const vk::Instance instance) const;

            void on_resize(std::function<void(const uvec2&)> callback);
            void on_key_press(std::function<void(const SDL_Keycode key)> callback);
            void on_key_release(std::function<void(const SDL_Keycode key)> callback);
            void on_mouse_move(std::function<void(const ivec2&)> callback);
            void on_wheel_move(std::function<void(const ivec2&)> callback);
            void on_button_press(std::function<void(const u8)> callback);
            void on_event(std::function<void(SDL_Event e)> callback);

            void set_capture_mouse(b8 capture);
            void set_title(const str& title);
            void set_resizable(const b8 resizable);
            void set_fullscreen(const u32 flags = 0);

            b8 is_key_pressed(const SDL_Keycode key);
            b8 is_key_down(const SDL_Keycode key);
            b8 is_button_down(const u8 button);
            b8 is_mouse_captured() const;
            b8 is_minimized() const;
            b8 is_flag_set(const u32 flag) const;

            ivec2 get_mouse_position() const;
            uvec2 get_size() const;
            SDL_Window* get_handle() const { return handle; };
            const std::vector<const char*>& get_instance_extensions() const { return extensions; };

        private:
            std::function<void(const vec2&)> resize = {};
            std::function<void(const SDL_Keycode key)> key_press = {};
            std::function<void(const SDL_Keycode key)> key_release = {};
            std::function<void(const vec2&)> mouse_move = {};
            std::function<void(const vec2&)> wheel_move = {};
            std::function<void(const u8 button)> button_press = {};
            std::function<void(SDL_Event e)> editor_events = {};

            SDL_Window* handle = {};
            u32 update_counter = {};
            b8 ignore_mouse_motion_events = {};
            std::vector<const char*> extensions;

            std::unordered_map<SDL_Keycode, b8> key_state;
            std::unordered_map<SDL_Keycode, u32> key_update;
            std::unordered_map<u8, b8> button_state;
            std::unordered_map<u8, u32> button_update;
    };
};  // namespace mag
