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

        enum class QueueType
        {
            Present,
            Graphics,
            Compute,
            Transfer
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

        enum class PrimitiveTopology
        {
            TriangleList
        };

        enum class CommandBufferLevel
        {
            Primary,
            Secondary
        };

        struct IFenceDesc
        {
                b8 signaled = false;
        };

        struct ISwapchainDesc
        {
                PresentMode desired_present_mode = PresentMode::Mailbox;
        };

        struct IQueueDesc
        {
                QueueType queue_type;
        };

        struct IGraphicsPipelineDesc
        {
                PrimitiveTopology primitive_topology;
                Format format;
                math::uvec2 extent;
        };

        struct ICommandBufferDesc
        {
                CommandBufferLevel command_buffer_level = CommandBufferLevel::Primary;
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

                virtual math::uvec2 get_extent() const = 0;

                virtual Format get_format() const = 0;

                virtual b8 acquire_next_image(const ISemaphore* signal_semaphore, const IFence* fence = nullptr) = 0;

                virtual b8 resize(const math::uvec2& extent) = 0;
        };

        class IQueue
        {
            public:
                virtual ~IQueue() = default;

                // @TODO: create command buffer interface instead of using void*
                virtual void submit(const ISemaphore* wait_semaphore, const ISemaphore* signal_semaphore, IFence* fence,
                                    void* command_buffer) = 0;

                virtual i32 present(const ISwapchain* swapchain, const ISemaphore* wait_semaphore) = 0;
        };

        class IGraphicsPipeline
        {
            public:
                virtual ~IGraphicsPipeline() = default;
        };

        class ICommandBuffer
        {
            public:
                virtual ~ICommandBuffer() = default;

                virtual void begin_recording() = 0;

                virtual void end_recording() = 0;

                virtual void set_viewport(const math::vec2& extent, const math::vec2& offset = {0.0f, 0.0f},
                                          const f32 min_depth = 0.0f, const f32 max_depth = 1.0f) = 0;

                virtual void set_scissor(const math::uvec2& extent, const math::ivec2& offset = {0.0f, 0.0f}) = 0;

                // @TODO: change when the interface is implemented
                virtual void begin_rendering(const void* render_info) = 0;

                virtual void end_rendering() = 0;

                virtual void bind_pipeline(const IGraphicsPipeline* pipeline) = 0;

                virtual void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                                  const u32 first_instance = 0) = 0;

                // @TODO: change parameters when the texture interface is implemented
                virtual void pipeline_barrier(const void* texture, const u32 old_layout, const u32 new_layout,
                                              const u32 src_access_mask, const u32 dst_access_mask,
                                              const u32 src_stage_mask, const u32 dst_stage_mask) = 0;
        };

        class IDevice
        {
            public:
                virtual ~IDevice() = default;

                virtual unique<ISemaphore> create_semaphore() = 0;
                virtual unique<IFence> create_fence(const IFenceDesc& desc) = 0;
                virtual unique<ISwapchain> create_swapchain(const ISwapchainDesc& desc) = 0;
                virtual unique<IQueue> create_queue(const IQueueDesc& desc) = 0;
                virtual unique<IGraphicsPipeline> create_graphics_pipeline(const IGraphicsPipelineDesc& desc) = 0;
                virtual unique<ICommandBuffer> create_command_buffer(const ICommandBufferDesc& desc) = 0;

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
