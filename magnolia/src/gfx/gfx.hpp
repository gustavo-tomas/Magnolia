#pragma once

#include "core/types.hpp"

namespace mag
{
    namespace gfx
    {
        // This is backend stuff

        struct IFenceDesc
        {
                b8 signaled = false;
        };

        class ISemaphore
        {
            public:
                virtual ~ISemaphore() = default;

                // @TODO: temp until the rest of the backend is completed
                virtual void* get_handle() = 0;
        };

        class IFence
        {
            public:
                virtual ~IFence() = default;

                virtual void wait(const u64 timeout = Timeout) = 0;

                virtual void reset() = 0;

                // @TODO: temp until the rest of the backend is completed
                virtual void* get_handle() = 0;
        };

        class IDevice
        {
            public:
                virtual ~IDevice() = default;

                virtual unique<ISemaphore> create_semaphore() = 0;
                virtual unique<IFence> create_fence(const IFenceDesc& desc) = 0;

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
