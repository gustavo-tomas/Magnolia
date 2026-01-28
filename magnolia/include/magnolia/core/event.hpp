#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    enum class Keys : u8;
    enum class Buttons : u8;

    struct MAG_API Event
    {
            virtual ~Event();
    };

    // Call the provided callback if T matches the event type
    template <typename T, typename F>
    void dispatch_event(const Event& event, const F& func)
    {
        if (const auto* e = dynamic_cast<const T*>(&event))
        {
            func(*e);
        }
    }

    struct WindowCloseEvent : public Event
    {
            // Empty
    };

    struct WindowResizeEvent : public Event
    {
            WindowResizeEvent(const u32 width, const u32 height);

            u32 width;
            u32 height;
    };

    struct KeyPressEvent : public Event
    {
            explicit KeyPressEvent(const Keys key);

            Keys key;
    };

    struct KeyReleaseEvent : public Event
    {
            explicit KeyReleaseEvent(const Keys key);

            Keys key;
    };

    struct MouseMoveEvent : public Event
    {
            MouseMoveEvent(const i32 x_direction, const i32 y_direction, const i32 x, const i32 y);

            i32 x_direction;
            i32 y_direction;
            i32 x;
            i32 y;
    };

    struct MouseScrollEvent : public Event
    {
            MouseScrollEvent(const f64 x_offset, const f64 y_offset);

            f64 x_offset;
            f64 y_offset;
    };

    struct MousePressEvent : public Event
    {
            explicit MousePressEvent(const Buttons button);

            Buttons button;
    };

    struct QuitEvent : public Event
    {
            // Empty
    };
};  // namespace mag
