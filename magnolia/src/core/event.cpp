#include "core/event.hpp"

#include "core/keys.hpp"

namespace mag
{
#define EVENT_CLASS_TYPE_DEFINITION(event, type)                   \
    EventType event::get_static_type() { return EventType::type; } \
    EventType event::get_type() const { return get_static_type(); }

    Event::~Event() = default;

    EventDispatcher::EventDispatcher(Event& event) : event(event) {}

    WindowResizeEvent::WindowResizeEvent(const u32 width, const u32 height) : width(width), height(height) {}

    KeyPressEvent::KeyPressEvent(const Key key) : key(key) {}

    KeyReleaseEvent::KeyReleaseEvent(const Key key) : key(key) {}

    MouseMoveEvent::MouseMoveEvent(const i32 x_direction, const i32 y_direction)
        : x_direction(x_direction), y_direction(y_direction)
    {
    }

    MouseScrollEvent::MouseScrollEvent(const f64 x_offset, const f64 y_offset) : x_offset(x_offset), y_offset(y_offset)
    {
    }

    MousePressEvent::MousePressEvent(const Button button) : button(button) {}

    NativeEvent::NativeEvent(const void* e) : e(e) {}

    EVENT_CLASS_TYPE_DEFINITION(WindowCloseEvent, WindowClose);
    EVENT_CLASS_TYPE_DEFINITION(WindowResizeEvent, WindowResize);
    EVENT_CLASS_TYPE_DEFINITION(KeyPressEvent, KeyPress);
    EVENT_CLASS_TYPE_DEFINITION(KeyReleaseEvent, KeyRelease);
    EVENT_CLASS_TYPE_DEFINITION(MouseMoveEvent, MouseMove);
    EVENT_CLASS_TYPE_DEFINITION(MouseScrollEvent, MouseScroll);
    EVENT_CLASS_TYPE_DEFINITION(MousePressEvent, MousePress);
    EVENT_CLASS_TYPE_DEFINITION(NativeEvent, NativeEvent);
    EVENT_CLASS_TYPE_DEFINITION(QuitEvent, Quit);
};  // namespace mag
