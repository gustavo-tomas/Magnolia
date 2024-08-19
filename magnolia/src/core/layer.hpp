#pragma once

#include "core/event.hpp"
#include "core/types.hpp"

namespace mag
{
    class Layer
    {
        public:
            virtual ~Layer() = default;

            virtual void on_attach(){};
            virtual void on_detach(){};
            virtual void on_update(const f32 dt) { (void)dt; };
            virtual void on_event(Event& e) { (void)e; };
    };
}  // namespace mag
