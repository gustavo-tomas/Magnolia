#pragma once

#include <functional>

#include "SDL_events.h"
#include "core/types.hpp"

namespace mag
{
#define EVENT_CLASS_TYPE(type) \
    static EventType GetStaticType() { return EventType::type; }

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
                if (event.get_type() == T::GetStaticType())
                {
                    func(static_cast<T&>(event));
                }
            }

        private:
            Event& event;
    };

    struct WindowCloseEvent : public Event
    {
            virtual EventType get_type() const override { return EventType::WindowClose; }

            // Empty
    };

    struct WindowResizeEvent : public Event
    {
            WindowResizeEvent(const u32 width, const u32 height) : width(width), height(height) {}

            virtual EventType get_type() const override { return EventType::WindowResize; }

            u32 width;
            u32 height;
    };

    struct KeyPressEvent : public Event
    {
            KeyPressEvent(const u32 key) : key(key) {}

            virtual EventType get_type() const override { return EventType::KeyPress; }

            u32 key;
    };

    struct KeyReleaseEvent : public Event
    {
            KeyReleaseEvent(const u32 key) : key(key) {}

            virtual EventType get_type() const override { return EventType::KeyRelease; }

            u32 key;
    };

    struct MouseMoveEvent : public Event
    {
            MouseMoveEvent(const i32 x_direction, const i32 y_direction)
                : x_direction(x_direction), y_direction(y_direction)
            {
            }

            virtual EventType get_type() const override { return EventType::MouseMove; }

            i32 x_direction;
            i32 y_direction;
    };

    struct MouseScrollEvent : public Event
    {
            MouseScrollEvent(const f64 x_offset, const f64 y_offset) : x_offset(x_offset), y_offset(y_offset) {}

            virtual EventType get_type() const override { return EventType::MouseScroll; }

            f64 x_offset;
            f64 y_offset;
    };

    struct MousePressEvent : public Event
    {
            MousePressEvent(const u8 button) : button(button) {}

            virtual EventType get_type() const override { return EventType::MousePress; }

            u8 button;
    };

    // @TODO: ooffff
    struct SDLEvent : public Event
    {
            SDLEvent(const SDL_Event e) : e(e) {}

            virtual EventType get_type() const override { return EventType::SDLEvent; }

            SDL_Event e;
    };
};  // namespace mag
