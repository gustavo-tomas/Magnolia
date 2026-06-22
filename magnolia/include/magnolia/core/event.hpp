#pragma once

#include "magnolia/core/types.hpp"

namespace mag
{
    // Avoid RTTI and all that
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define STATIC_EVENT_TYPE(event_type)                    \
    static constexpr EventType static_type = event_type; \
    EventType type() const override { return static_type; }

    enum class Keys : u8;
    enum class Buttons : u8;

    enum class EventType : u8
    {
        WindowCloseEvent,
        WindowResizeEvent,
        KeyPressEvent,
        KeyReleaseEvent,
        MouseMoveEvent,
        MouseScrollEvent,
        MousePressEvent,
        QuitEvent
    };  // namespace mag

    struct MAG_API Event
    {
            virtual ~Event();

            virtual EventType type() const = 0;
    };

    // Call the provided callback if T matches the event type
    template <typename T, typename F>
    void dispatch_event(const Event& event, const F& func)
    {
        if (event.type() == T::static_type)
        {
            func(static_cast<const T&>(event));
        }
    }

    struct WindowCloseEvent : public Event
    {
            STATIC_EVENT_TYPE(EventType::WindowCloseEvent);
    };

    struct WindowResizeEvent : public Event
    {
            WindowResizeEvent(u32 width, u32 height);

            STATIC_EVENT_TYPE(EventType::WindowResizeEvent);

            u32 width;
            u32 height;
    };

    struct KeyPressEvent : public Event
    {
            explicit KeyPressEvent(Keys key);

            STATIC_EVENT_TYPE(EventType::KeyPressEvent);

            Keys key;
    };

    struct KeyReleaseEvent : public Event
    {
            explicit KeyReleaseEvent(Keys key);

            STATIC_EVENT_TYPE(EventType::KeyReleaseEvent);

            Keys key;
    };

    struct MouseMoveEvent : public Event
    {
            MouseMoveEvent(i32 x_direction, i32 y_direction, i32 x, i32 y);

            STATIC_EVENT_TYPE(EventType::MouseMoveEvent);

            i32 x_direction;
            i32 y_direction;
            i32 x;
            i32 y;
    };

    struct MouseScrollEvent : public Event
    {
            MouseScrollEvent(f64 x_offset, f64 y_offset);

            STATIC_EVENT_TYPE(EventType::MouseScrollEvent);

            f64 x_offset;
            f64 y_offset;
    };

    struct MousePressEvent : public Event
    {
            explicit MousePressEvent(Buttons button);

            STATIC_EVENT_TYPE(EventType::MousePressEvent);

            Buttons button;
    };

    struct QuitEvent : public Event
    {
            STATIC_EVENT_TYPE(EventType::QuitEvent);
    };
};  // namespace mag
