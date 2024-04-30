#pragma once

#include <functional>
#include <map>

#include "SDL_events.h"
#include "core/logger.hpp"
#include "core/types.hpp"

namespace mag
{
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
    };

    class EventManager
    {
        public:
            // @TODO: dynamic casting pls tyty
            using Callback = std::function<void(Event&)>;

            void subscribe(const EventType event_type, const Callback& callback)
            {
                callbacks[event_type].push_back(callback);
            }

            void emit(const EventType event_type, Event& event)
            {
                auto it = callbacks.find(event_type);
                if (it != callbacks.end())
                {
                    auto& registered_callbacks = it->second;
                    for (auto& callback : registered_callbacks)
                    {
                        callback(event);
                    }
                }

                else
                {
                    LOG_WARNING("No registered callback for type: {0}", static_cast<u32>(event_type));
                }
            }

        private:
            std::map<EventType, std::vector<Callback>> callbacks;
    };

    struct WindowCloseEvent : public Event
    {
            // Empty
    };

    struct WindowResizeEvent : public Event
    {
            WindowResizeEvent(const u32 width, const u32 height) : width(width), height(height) {}

            u32 width;
            u32 height;
    };

    struct KeyPressEvent : public Event
    {
            KeyPressEvent(const u32 key) : key(key) {}

            u32 key;
    };

    struct KeyReleaseEvent : public Event
    {
            KeyReleaseEvent(const u32 key) : key(key) {}

            u32 key;
    };

    struct MouseMoveEvent : public Event
    {
            MouseMoveEvent(const i32 x_direction, const i32 y_direction)
                : x_direction(x_direction), y_direction(y_direction)
            {
            }

            i32 x_direction;
            i32 y_direction;
    };

    struct MouseScrollEvent : public Event
    {
            MouseScrollEvent(const f64 x_offset, const f64 y_offset) : x_offset(x_offset), y_offset(y_offset) {}

            f64 x_offset;
            f64 y_offset;
    };

    struct MousePressEvent : public Event
    {
            MousePressEvent(const u8 button) : button(button) {}

            u8 button;
    };

    // @TODO: ooffff
    struct SDLEvent : public Event
    {
            SDLEvent(const SDL_Event e) : e(e) {}

            SDL_Event e;
    };
};  // namespace mag
