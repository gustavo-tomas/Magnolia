#pragma once

#include <functional>

#include "core/event.hpp"
#include "core/keys.hpp"
#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    using EventCallback = std::function<void(const Event&)>;

    struct WindowOptions
    {
            static constexpr math::uvec2 MaxSize = math::uvec2(Max_U32);
            static constexpr math::ivec2 CenterPos = math::ivec2(Max_I32);

            const EventCallback& event_callback;
            math::uvec2 size = MaxSize;
            math::ivec2 position = CenterPos;
            str title = "Magnolia";
            str window_icon = "";
    };

    namespace window
    {
        b8 initialize(const WindowOptions& options);
        void shutdown();

        void on_update();

        void create_surface(const void* instance, void* surface);

        b8 set_window_icon(const str& bmp_file);
        void set_title(const str& title);
        void set_resizable(const b8 resizable);
        void set_fullscreen(const b8 fullscreen);

        MAG_API void set_capture_mouse(b8 capture);

        MAG_API b8 is_key_pressed(const Key key);
        MAG_API b8 is_key_down(const Key key);
        MAG_API b8 is_button_pressed(const Button button);
        MAG_API b8 is_button_down(const Button button);
        MAG_API b8 is_mouse_captured();
        MAG_API b8 is_minimized();
        MAG_API b8 is_fullscreen();

        MAG_API math::ivec2 get_mouse_position();
        MAG_API math::uvec2 get_size();

        const std::vector<const c8*>& get_instance_extensions();
    };  // namespace window
};      // namespace mag
