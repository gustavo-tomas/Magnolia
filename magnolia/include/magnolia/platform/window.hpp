#pragma once

#include <functional>

#include "magnolia/core/event.hpp"
#include "magnolia/core/keys.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    namespace window
    {
        using EventCallback = std::function<void(const Event&)>;

        struct WindowOptions
        {
                static constexpr u32 MaxSize = Max_U32;
                static constexpr i32 CenterPos = Max_I32;

                math::uvec2 size = math::uvec2(MaxSize);
                math::ivec2 position = math::ivec2(CenterPos);
                str title = "Magnolia";
                str window_icon;
                i32 target_frame_rate = -1;
        };

        b8 initialize(const WindowOptions& options);
        void shutdown();

        void create_surface(const void* instance, void* surface);

        MAG_API void on_update();

        MAG_API b8 set_window_icon(const str& bmp_file);
        MAG_API void set_title(const str& title);
        MAG_API void set_resizable(const b8 resizable);
        MAG_API void set_fullscreen(const b8 fullscreen);
        MAG_API void set_event_callback(const EventCallback& callback);

        MAG_API void set_capture_mouse(const b8 capture);
        MAG_API void set_mouse_position(const i32 x, const i32 y);

        // Set target fps. -1 is no limits
        MAG_API void set_target_frame_rate(const i32 frame_rate = -1);

        MAG_API b8 is_key_pressed(const Key key);
        MAG_API b8 is_key_down(const Key key);
        MAG_API b8 is_button_pressed(const Button button);
        MAG_API b8 is_button_down(const Button button);
        MAG_API b8 is_mouse_captured();
        MAG_API b8 is_minimized();
        MAG_API b8 is_fullscreen();

        MAG_API math::ivec2 get_mouse_position();
        MAG_API math::uvec2 get_window_center();
        MAG_API math::uvec2 get_size();
        MAG_API f64 get_delta_time();

        const std::vector<const c8*>& get_instance_extensions();
    };  // namespace window
};  // namespace mag
