#pragma once

#include <functional>

#include "core/event.hpp"
#include "core/keys.hpp"
#include "core/math.hpp"
#include "core/types.hpp"

namespace mag
{
    using namespace mag::math;
    using EventCallback = std::function<void(Event&)>;

    struct WindowOptions
    {
            static constexpr uvec2 MAX_SIZE = uvec2(MAX_U32);
            static constexpr ivec2 CENTER_POS = ivec2(MAX_I32);

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

            void create_surface(const void* instance, void* surface) const;

            void sleep(const u32 ms);

            b8 set_window_icon(const str& bmp_file) const;
            void set_capture_mouse(b8 capture);
            void set_title(const str& title);
            void set_resizable(const b8 resizable);
            void set_fullscreen(const b8 fullscreen);

            b8 is_key_pressed(const Key key) const;
            b8 is_key_down(const Key key) const;
            b8 is_button_pressed(const Button button) const;
            b8 is_button_down(const Button button) const;
            b8 is_mouse_captured() const;
            b8 is_minimized() const;
            b8 is_fullscreen() const;

            ivec2 get_mouse_position() const;
            uvec2 get_size() const;
            f64 get_time() const;  // Ms since start
            void* get_handle() const;
            const std::vector<const c8*>& get_instance_extensions() const;

        private:
            b8 is_flag_set(const u32 flag) const;

            struct IMPL;
            unique<IMPL> impl;
    };
};  // namespace mag
