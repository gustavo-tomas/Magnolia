#pragma once

#include "core/types.hpp"

namespace mag
{
    namespace gfx
    {
        // This is backend stuff
        class IDevice
        {
            public:
                virtual ~IDevice() = default;

                // @TODO: temporary stub to draw stuff
                virtual void draw_frame() = 0;
        };

        unique<IDevice> create_device();

        // This is front-end stuff
        b8 initialize();
        void shutdown();

        void on_update(const f32 dt);
    };  // namespace gfx
};      // namespace mag
