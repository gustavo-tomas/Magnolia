#pragma once

#include <functional>

#include "SDL_events.h"
#include "core/types.hpp"

namespace mag
{
    // Dark magic
#define EVENT_CLASS_TYPE(type)                                     \
    static EventType get_static_type() { return EventType::type; } \
    virtual EventType get_type() const override { return get_static_type(); }

    enum class EventType
    {
        WindowClose = 0,
        WindowResize,
        KeyPress,
        KeyRelease,
        MouseMove,
        MouseScroll,
        MousePress,
        SDLEvent
    };

    struct Event
    {
            virtual ~Event() {}

            virtual EventType get_type() const = 0;
    };

    using EventCallback = std::function<void(Event&)>;

    // Automatic type deduction
    // https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Events/Event.h
    class EventDispatcher
    {
        public:
            EventDispatcher(Event& event) : event(event) {}

            // F will be deduced by the compiler
            template <typename T, typename F>
            void dispatch(const F& func)
            {
                if (event.get_type() == T::get_static_type())
                {
                    func(static_cast<T&>(event));
                }
            }

        private:
            Event& event;
    };

    struct WindowCloseEvent : public Event
    {
            EVENT_CLASS_TYPE(WindowClose);

            // Empty
    };

    struct WindowResizeEvent : public Event
    {
            WindowResizeEvent(const u32 width, const u32 height) : width(width), height(height) {}

            EVENT_CLASS_TYPE(WindowResize);

            u32 width;
            u32 height;
    };

    struct KeyPressEvent : public Event
    {
            KeyPressEvent(const u32 key) : key(key) {}

            EVENT_CLASS_TYPE(KeyPress);

            u32 key;
    };

    struct KeyReleaseEvent : public Event
    {
            KeyReleaseEvent(const u32 key) : key(key) {}

            EVENT_CLASS_TYPE(KeyRelease);

            u32 key;
    };

    struct MouseMoveEvent : public Event
    {
            MouseMoveEvent(const i32 x_direction, const i32 y_direction)
                : x_direction(x_direction), y_direction(y_direction)
            {
            }

            EVENT_CLASS_TYPE(MouseMove);

            i32 x_direction;
            i32 y_direction;
    };

    struct MouseScrollEvent : public Event
    {
            MouseScrollEvent(const f64 x_offset, const f64 y_offset) : x_offset(x_offset), y_offset(y_offset) {}

            EVENT_CLASS_TYPE(MouseScroll);

            f64 x_offset;
            f64 y_offset;
    };

    struct MousePressEvent : public Event
    {
            MousePressEvent(const u8 button) : button(button) {}

            EVENT_CLASS_TYPE(MousePress);

            u8 button;
    };

    // @TODO: ooffff
    struct SDLEvent : public Event
    {
            SDLEvent(const SDL_Event e) : e(e) {}

            EVENT_CLASS_TYPE(SDLEvent);

            SDL_Event e;
    };
};  // namespace mag
