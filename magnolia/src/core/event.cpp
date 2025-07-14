#include "magnolia/core/event.hpp"

#include "magnolia/core/keys.hpp"

namespace mag
{
    Event::~Event() = default;

    WindowResizeEvent::WindowResizeEvent(const u32 width, const u32 height) : width(width), height(height) {}

    KeyPressEvent::KeyPressEvent(const Key key) : key(key) {}

    KeyReleaseEvent::KeyReleaseEvent(const Key key) : key(key) {}

    MouseMoveEvent::MouseMoveEvent(const i32 x_direction, const i32 y_direction, const i32 x, const i32 y)
        : x_direction(x_direction), y_direction(y_direction), x(x), y(y)
    {
    }

    MouseScrollEvent::MouseScrollEvent(const f64 x_offset, const f64 y_offset) : x_offset(x_offset), y_offset(y_offset)
    {
    }

    MousePressEvent::MousePressEvent(const Button button) : button(button) {}
};  // namespace mag
