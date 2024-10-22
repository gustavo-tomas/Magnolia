#pragma once

#include "core/keys.hpp"
#include "core/types.hpp"

namespace mag
{
    // Dark magic
#define EVENT_CLASS_TYPE_DECLARATION    \
    static EventType get_static_type(); \
    virtual EventType get_type() const override;

    enum class EventType
    {
        // Window events
        WindowClose = 0,
        WindowResize,
        KeyPress,
        KeyRelease,
        MouseMove,
        MouseScroll,
        MousePress,
        NativeEvent,

        // Client events
        Quit
    };

    struct Event
    {
            virtual ~Event();

            virtual EventType get_type() const = 0;
    };

    // Automatic type deduction
    // https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Events/Event.h
    class EventDispatcher
    {
        public:
            explicit EventDispatcher(Event& event);

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
            EVENT_CLASS_TYPE_DECLARATION;

            // Empty
    };

    struct WindowResizeEvent : public Event
    {
            WindowResizeEvent(const u32 width, const u32 height);

            EVENT_CLASS_TYPE_DECLARATION;

            u32 width;
            u32 height;
    };

    struct KeyPressEvent : public Event
    {
            explicit KeyPressEvent(const Key key);

            EVENT_CLASS_TYPE_DECLARATION;

            Key key;
    };

    struct KeyReleaseEvent : public Event
    {
            explicit KeyReleaseEvent(const Key key);

            EVENT_CLASS_TYPE_DECLARATION;

            Key key;
    };

    struct MouseMoveEvent : public Event
    {
            MouseMoveEvent(const i32 x_direction, const i32 y_direction);

            EVENT_CLASS_TYPE_DECLARATION;

            i32 x_direction;
            i32 y_direction;
    };

    struct MouseScrollEvent : public Event
    {
            MouseScrollEvent(const f64 x_offset, const f64 y_offset);

            EVENT_CLASS_TYPE_DECLARATION;

            f64 x_offset;
            f64 y_offset;
    };

    struct MousePressEvent : public Event
    {
            explicit MousePressEvent(const Button button);

            EVENT_CLASS_TYPE_DECLARATION;

            Button button;
    };

    struct NativeEvent : public Event
    {
            explicit NativeEvent(const void* e);

            EVENT_CLASS_TYPE_DECLARATION;

            const void* e;
    };

    struct QuitEvent : public Event
    {
            EVENT_CLASS_TYPE_DECLARATION;

            // Empty
    };
};  // namespace mag
