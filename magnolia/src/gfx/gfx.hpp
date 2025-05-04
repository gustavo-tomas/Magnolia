#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    namespace gfx
    {
        // This is backend stuff

        enum class PresentMode
        {
            Immediate,
            Mailbox,
            Fifo
        };

        enum class Format
        {
            R8G8B8A8_UNORM,
            B8G8R8A8_UNORM,
            B8G8R8A8_SRGB,
            R16G16B16A16_SFLOAT,
            R32G32B32A32_SFLOAT,
            D32_SFLOAT,
            D24_UNORM_S8_UINT
        };

        enum class TextureType
        {
            Texture1D,
            Texture2D,
            Texture3D,
            TextureCube
        };

        enum class TextureUsage : u32
        {
            TransferSrc = 0 << 1,
            TransferDst = 0 << 2,
            Sampled = 0 << 3,
            Storage = 0 << 4,
            ColorAttachment = 0 << 5,
            DepthStencilAttachment = 0 << 6
        };

        struct IFenceDesc
        {
                b8 signaled = false;
        };

        struct ISwapchainDesc
        {
                PresentMode desired_present_mode = PresentMode::Mailbox;
        };

        class ISemaphore
        {
            public:
                virtual ~ISemaphore() = default;
        };

        class IFence
        {
            public:
                virtual ~IFence() = default;

                virtual void wait(const u64 timeout = Timeout) = 0;

                virtual void reset() = 0;
        };

        class ISwapchain
        {
            public:
                virtual ~ISwapchain() = default;

                virtual u32 get_current_image_index() const = 0;

                virtual u32 get_image_count() const = 0;

                virtual math::vec2 get_extent() const = 0;

                virtual Format get_format() const = 0;

                virtual b8 acquire_next_image(const ISemaphore* signal_semaphore, const IFence* fence = nullptr) = 0;

                virtual b8 resize(const math::vec2& extent) = 0;
        };

        class IDevice
        {
            public:
                virtual ~IDevice() = default;

                virtual unique<ISemaphore> create_semaphore() = 0;
                virtual unique<IFence> create_fence(const IFenceDesc& desc) = 0;
                virtual unique<ISwapchain> create_swapchain(const ISwapchainDesc& desc) = 0;

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

ENABLE_BITMASK_OPERATORS(mag::gfx::TextureUsage);
