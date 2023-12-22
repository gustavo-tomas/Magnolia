#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>

#include <functional>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "core/math.hpp"
#include "core/types.hpp"

namespace mag
{
    struct WindowOptions
    {
            uvec2 size = {1024, 600};
            ivec2 position = {SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED};
            str title = "BlossV";
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

            void set_capture_mouse(b8 capture);
            void set_title(const str& title);
            void set_resizable(const b8 resizable);

            b8 is_key_pressed(const SDL_Keycode key);
            b8 is_key_down(const SDL_Keycode key);
            b8 is_mouse_captured() const;
            b8 is_minimized() const;
            ivec2 get_mouse_position() const;
            uvec2 get_size() const;
            const std::vector<const char*>& get_instance_extensions() const { return extensions; };

        private:
            std::function<void(const vec2&)> resize = {};
            std::function<void(const SDL_Keycode key)> key_press = {};
            std::function<void(const SDL_Keycode key)> key_release = {};
            std::function<void(const vec2&)> mouse_move = {};

            SDL_Window* handle = {};
            u32 update_counter = {};
            b8 ignore_mouse_motion_events = {};
            std::vector<const char*> extensions;

            std::unordered_map<SDL_Keycode, b8> key_state;
            std::unordered_map<SDL_Keycode, u32> key_update;
    };
};  // namespace mag
