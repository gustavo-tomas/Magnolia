#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    namespace gfx
    {
        class IRenderingAttachment;
        class ITexture;
        class ICommandPool;
        class IDescriptorPool;
        class IDescriptorSetLayout;
        class IDescriptorSet;

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
            Texture3D
        };

        enum class TextureViewType
        {
            Texture1D,
            Texture2D,
            Texture3D,
            TextureCube,
            Texture1DArray,
            Texture2DArray,
            TextureCubeArray
        };

        enum class TextureUsage : u32
        {
            TransferSrc = 1 << 0,
            TransferDst = 1 << 1,
            Sampled = 1 << 2,
            Storage = 1 << 3,
            ColorAttachment = 1 << 4,
            DepthStencilAttachment = 1 << 5
        };

        enum class TextureLayout
        {
            Undefined,
            ColorAttachment,
            Present,
            TransferSrc,
            TransferDst
        };

        enum class TextureAspect
        {
            None = 0,
            Color = 1 << 0,
            Depth = 1 << 1,
            Stencil = 1 << 2
        };

        enum class SampleCount
        {
            e1 = 1 << 0,
            e2 = 1 << 1,
            e4 = 1 << 2,
            e8 = 1 << 3,
            e16 = 1 << 4
        };

        enum class AccessMask
        {
            None = 0,
            ColorAttachmentWrite = 1 << 0,
            TransferRead = 1 << 1,
            TransferWrite = 1 << 2,
            MemoryRead = 1 << 3,
            MemoryWrite = 1 << 4
        };

        enum class PipelineStage
        {
            TopOfPipe = 1 << 0,
            ColorAttachmentOutput = 1 << 1,
            BottomOfPipe = 1 << 2,
            Transfer = 1 << 3,
            AllCommands = 1 << 4
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

        enum class RenderingAttachmentType
        {
            Color,
            Depth
        };

        enum class ShaderStage
        {
            Vertex = 1 << 0,
            Fragment = 1 << 1
        };

        enum class BufferUsage
        {
            Vertex = 1 << 0,
            Index = 1 << 1,
            Uniform = 1 << 2,
            Storage = 1 << 3,
            TransferSrc = 1 << 4,
            TransferDst = 1 << 5
        };

        enum class MemoryUsage
        {
            Auto,
            PreferHost,
            PreferDevice
        };

        enum class DescriptorType
        {
            Uniform,
            Storage,
            CombinedImageSampler
        };

        struct IShaderModuleDesc
        {
                ShaderStage stage;
                std::vector<u8> code;
        };

        struct IFenceDesc
        {
                b8 signaled = false;
        };

        struct ISemaphoreDesc
        {
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
                std::vector<IShaderModuleDesc> shader_modules;
                std::vector<const IDescriptorSetLayout*> descriptor_layouts;
                Format format;
                math::uvec2 extent;
        };

        struct ICommandPoolDesc
        {
                QueueType queue_type;
        };

        struct ICommandBufferDesc
        {
                CommandBufferLevel command_buffer_level = CommandBufferLevel::Primary;
                const ICommandPool* command_pool = nullptr;
        };

        struct IRenderingAttachmentDesc
        {
                math::vec4 clear_color = {1.0f, 1.0f, 1.0f, 1.0f};
                f32 clear_depth;
                u32 clear_stencil;
                RenderingAttachmentType type;
                const ITexture* texture = nullptr;
        };

        struct IRenderPassDesc
        {
                math::uvec2 extent;
                math::ivec2 offset = {0.0f, 0.0f};
                std::vector<IRenderingAttachment*> color_attachments;
        };

        struct ITextureDesc
        {
                math::uvec3 extent = {1.0f, 1.0f, 1.0f};
                Format format = Format::B8G8R8A8_SRGB;
                TextureType type = TextureType::Texture2D;
                TextureViewType view_type = TextureViewType::Texture2D;
                TextureAspect aspect = TextureAspect::Color;
                TextureUsage usage = TextureUsage::ColorAttachment;
                TextureLayout layout = TextureLayout::Undefined;
                u32 mip_levels = 1;
                u32 array_layers = 1;
                SampleCount sample_count = SampleCount::e1;
        };

        struct IBufferDesc
        {
                u64 size_bytes;
                BufferUsage buffer_usage;
                MemoryUsage memory_usage;
        };

        struct IDescriptorPoolSizeDesc
        {
                u32 size;
                DescriptorType type;
        };

        struct IDescriptorPoolDesc
        {
                std::vector<IDescriptorPoolSizeDesc> size_descs;
                u32 max_sets;
        };

        struct IDescriptorSetLayoutBindingDesc
        {
                u32 binding;
                u32 descriptor_count;
                DescriptorType descriptor_type;
                ShaderStage stages;
        };

        struct IDescriptorSetLayoutDesc
        {
                std::vector<IDescriptorSetLayoutBindingDesc> binding_descs;
        };

        struct IDescriptorSetDesc
        {
                const IDescriptorPool* descriptor_pool = nullptr;
                const IDescriptorSetLayout* descriptor_layout = nullptr;
                u32 max_descriptor_count;
        };

        class IBuffer
        {
            public:
                virtual ~IBuffer() = default;

                virtual void* map() = 0;

                virtual void unmap() = 0;

                virtual void set_data(const void* data, const u64 size, const u64 offset = 0) = 0;

                virtual u64 get_size() const = 0;

                virtual BufferUsage get_usage() const = 0;
        };

        class IRenderingAttachment
        {
            public:
                virtual ~IRenderingAttachment() = default;

                virtual math::vec4 get_clear_color() const = 0;

                virtual f32 get_clear_depth() const = 0;

                virtual u32 get_clear_stencil() const = 0;
        };

        class IRenderPass
        {
            public:
                virtual ~IRenderPass() = default;

                virtual math::ivec2 get_offset() const = 0;

                virtual math::uvec2 get_extent() const = 0;
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

        class ITexture
        {
            public:
                virtual ~ITexture() = default;

                virtual const math::uvec3& get_extent() const = 0;

                virtual Format get_format() const = 0;

                virtual TextureLayout get_layout() const = 0;

                virtual TextureType get_type() const = 0;

                virtual TextureViewType get_view_type() const = 0;

                virtual TextureAspect get_aspect() const = 0;

                virtual TextureUsage get_usage() const = 0;

                virtual SampleCount get_sample_count() const = 0;

                virtual u32 get_mip_levels() const = 0;

                virtual u32 get_array_layers() const = 0;
        };

        class ISwapchain
        {
            public:
                virtual ~ISwapchain() = default;

                virtual u32 get_current_image_index() const = 0;

                virtual u32 get_image_count() const = 0;

                virtual math::uvec2 get_extent() const = 0;

                virtual Format get_format() const = 0;

                virtual const ITexture* get_texture(const u32 index) const = 0;

                virtual b8 acquire_next_image(const ISemaphore* signal_semaphore, const IFence* fence = nullptr) = 0;

                virtual b8 resize(const math::uvec2& extent) = 0;
        };

        class IDescriptorPool
        {
            public:
                virtual ~IDescriptorPool() = default;
        };

        class IDescriptorSetLayout
        {
            public:
                virtual ~IDescriptorSetLayout() = default;
        };

        class IDescriptorSet
        {
            public:
                virtual ~IDescriptorSet() = default;

                virtual void update(const IBuffer* const buffer, const u32 binding, const u32 array_element,
                                    const DescriptorType descriptor_type, const u64 offset = 0) = 0;
        };

        class IGraphicsPipeline
        {
            public:
                virtual ~IGraphicsPipeline() = default;
        };

        class ICommandPool
        {
            public:
                virtual ~ICommandPool() = default;
        };

        class ICommandBuffer
        {
            public:
                virtual ~ICommandBuffer() = default;

                virtual void begin_recording() = 0;

                virtual void end_recording() = 0;

                virtual void reset() = 0;

                virtual void set_viewport(const math::vec2& extent, const math::vec2& offset = {0.0f, 0.0f},
                                          const f32 min_depth = 0.0f, const f32 max_depth = 1.0f) = 0;

                virtual void set_scissor(const math::uvec2& extent, const math::ivec2& offset = {0.0f, 0.0f}) = 0;

                virtual void begin_rendering(const IRenderPass* render_pass) = 0;

                virtual void end_rendering() = 0;

                virtual void bind_pipeline(const IGraphicsPipeline* pipeline) = 0;

                virtual void bind_descriptor(const IGraphicsPipeline* pipeline, const IDescriptorSet* descriptor) = 0;

                virtual void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                                  const u32 first_instance = 0) = 0;

                virtual void pipeline_barrier(const ITexture* texture, const TextureLayout new_layout,
                                              const AccessMask src_access_mask, const AccessMask dst_access_mask,
                                              const PipelineStage src_stage_mask,
                                              const PipelineStage dst_stage_mask) = 0;

                virtual void copy_texture(const ITexture* src_texture, const ITexture* dst_texture) = 0;
        };

        class IQueue
        {
            public:
                virtual ~IQueue() = default;

                virtual void submit(const ISemaphore* wait_semaphore, const ISemaphore* signal_semaphore, IFence* fence,
                                    const ICommandBuffer* command_buffer) = 0;

                virtual i32 present(const ISwapchain* swapchain, const ISemaphore* wait_semaphore) = 0;
        };

        class IDevice
        {
            public:
                virtual ~IDevice() = default;

                virtual void wait_idle() = 0;

                virtual unique<ISemaphore> create_semaphore(const ISemaphoreDesc& desc) = 0;

                virtual unique<IFence> create_fence(const IFenceDesc& desc) = 0;

                virtual unique<ISwapchain> create_swapchain(const ISwapchainDesc& desc) = 0;

                virtual unique<IQueue> create_queue(const IQueueDesc& desc) = 0;

                virtual unique<IGraphicsPipeline> create_graphics_pipeline(const IGraphicsPipelineDesc& desc) = 0;

                virtual unique<ICommandPool> create_command_pool(const ICommandPoolDesc& desc) = 0;

                virtual unique<ICommandBuffer> create_command_buffer(const ICommandBufferDesc& desc) = 0;

                virtual unique<IRenderingAttachment> create_render_attachment(const IRenderingAttachmentDesc& desc) = 0;

                virtual unique<IRenderPass> create_render_pass(const IRenderPassDesc& desc) = 0;

                virtual unique<ITexture> create_texture(const ITextureDesc& desc) = 0;

                virtual unique<IBuffer> create_buffer(const IBufferDesc& desc) = 0;

                virtual unique<IDescriptorPool> create_descriptor_pool(const IDescriptorPoolDesc& desc) = 0;

                virtual unique<IDescriptorSetLayout> create_descriptor_set_layout(
                    const IDescriptorSetLayoutDesc& desc) = 0;

                virtual unique<IDescriptorSet> create_descriptor_set(const IDescriptorSetDesc& desc) = 0;
        };

        unique<IDevice> create_device();
    };  // namespace gfx
};      // namespace mag

ENABLE_BITMASK_OPERATORS(mag::gfx::BufferUsage);
ENABLE_BITMASK_OPERATORS(mag::gfx::TextureUsage);
ENABLE_BITMASK_OPERATORS(mag::gfx::TextureAspect);
ENABLE_BITMASK_OPERATORS(mag::gfx::AccessMask);
ENABLE_BITMASK_OPERATORS(mag::gfx::PipelineStage);
ENABLE_BITMASK_OPERATORS(mag::gfx::ShaderStage);
