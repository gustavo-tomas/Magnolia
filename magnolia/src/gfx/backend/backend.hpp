#pragma once

#include <functional>

#include "magnolia/core/types.hpp"
#include "magnolia/gfx/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag::gfx
{
    class IRenderingAttachment;
    class ITexture;
    class ICommandPool;
    class IDescriptorPool;
    class IDescriptorSetLayout;
    class IDescriptorSet;

    struct IShaderModuleDesc
    {
            ShaderStage stage = ShaderStage::Vertex;
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
            math::uvec2 desired_extent = math::uvec2(256, 256);
    };

    struct IQueueDesc
    {
            QueueType queue_type = QueueType::Graphics;
    };

    struct IVertexAttributeDesc
    {
            Format format = Format::Undefined;
            u32 binding = 0;
            u32 location = 0;
            u32 offset = 0;
    };

    struct IVertexBindingDesc
    {
            u32 binding = 0;
            u32 stride = 0;
            VertexInputRate input_rate = VertexInputRate::Vertex;
    };

    struct IGraphicsPipelineColorBlend
    {
            b8 blend_enable = false;
            BlendOp color_blend_op = BlendOp::Add;
            BlendOp alpha_blend_op = BlendOp::Add;
            BlendFactor src_color_blend_factor = BlendFactor::SrcAlpha;
            BlendFactor dst_color_blend_factor = BlendFactor::OneMinusSrcAlpha;
            BlendFactor src_alpha_blend_factor = BlendFactor::One;
            BlendFactor dst_alpha_blend_factor = BlendFactor::OneMinusSrcAlpha;
    };

    struct IGraphicsPipelineDesc
    {
            PrimitiveTopology primitive_topology = PrimitiveTopology::TriangleList;
            std::vector<IShaderModuleDesc> shader_modules;
            std::vector<const IDescriptorSetLayout*> descriptor_layouts;
            std::vector<IVertexAttributeDesc> vertex_attribute_descs;
            std::vector<IVertexBindingDesc> vertex_binding_descs;
            Format color_attachment_format = Format::Undefined;
            Format depth_attachment_format = Format::Undefined;
            math::uvec2 extent = {0, 0};
            IGraphicsPipelineColorBlend color_blend;
    };

    struct ICommandPoolDesc
    {
            QueueType queue_type = QueueType::Graphics;
    };

    struct ICommandBufferDesc
    {
            CommandBufferLevel command_buffer_level = CommandBufferLevel::Primary;
            const ICommandPool* command_pool = nullptr;
    };

    struct IRenderingAttachmentDesc
    {
            math::vec4 clear_color = {1.0F, 1.0F, 1.0F, 1.0F};
            f32 clear_depth = 0.0F;
            u32 clear_stencil = 0;
            RenderingAttachmentType type = RenderingAttachmentType::Color;
            const ITexture* texture = nullptr;
    };

    struct IRenderPassDesc
    {
            math::uvec2 extent = {0, 0};
            math::ivec2 offset = {0, 0};
            std::vector<const IRenderingAttachment*> color_attachments;
            const IRenderingAttachment* depth_attachment = nullptr;
    };

    struct ITextureDesc
    {
            math::uvec3 extent = {1, 1, 1};
            Format format = Format::B8G8R8A8_SRGB;
            TextureType type = TextureType::Texture2D;
            TextureViewType view_type = TextureViewType::Texture2D;
            TextureAspect aspect = TextureAspect::Color;
            TextureUsage usage = TextureUsage::ColorAttachment;
            u32 mip_levels = 1;
            u32 array_layers = 1;
            SampleCount sample_count = SampleCount::e1;
    };

    struct IBufferDesc
    {
            u64 size_bytes = 0;
            BufferUsage buffer_usage = BufferUsage::Vertex;
            MemoryUsage memory_usage = MemoryUsage::Auto;
    };

    struct IDescriptorPoolSizeDesc
    {
            u32 count = 0;
            DescriptorType type = DescriptorType::Uniform;
    };

    struct IDescriptorPoolDesc
    {
            std::vector<IDescriptorPoolSizeDesc> size_descs;
            u32 max_sets = 0;
    };

    struct IDescriptorSetLayoutBindingDesc
    {
            u32 binding = 0;
            u32 descriptor_count = 0;
            b8 variable_descriptor_count = false;
            DescriptorType descriptor_type = DescriptorType::Uniform;
            ShaderStage stages = ShaderStage::Vertex;
    };

    struct IDescriptorSetLayoutDesc
    {
            std::vector<IDescriptorSetLayoutBindingDesc> binding_descs;
    };

    struct IDescriptorSetDesc
    {
            const IDescriptorPool* descriptor_pool = nullptr;
            const IDescriptorSetLayout* descriptor_layout = nullptr;
            u32 max_variable_descriptor_count = 0;
    };

    struct ISamplerDesc
    {
            Filter min_filter = Filter::Linear;
            Filter mag_filter = Filter::Linear;
            SamplerMipMapMode mipmap_mode = SamplerMipMapMode::Linear;
            SamplerAddressMode address_mode_u = SamplerAddressMode::Repeat;
            SamplerAddressMode address_mode_v = SamplerAddressMode::Repeat;
            SamplerAddressMode address_mode_w = SamplerAddressMode::Repeat;
            f32 min_lod = 0.0F;
            f32 max_lod = 0.0F;
            b8 anisotropy_enable = false;
            f32 max_anisotropy = 0.0F;
    };

    struct DescriptorLimits
    {
            u32 max_per_stage_combined_image_samplers = 0;
            u32 max_per_stage_uniform_buffers = 0;
            u32 max_per_stage_storage_buffers = 0;
    };

    class ISampler
    {
        public:
            virtual ~ISampler() = default;
    };

    class IBuffer
    {
        public:
            virtual ~IBuffer() = default;

            virtual void* map() = 0;

            virtual void unmap() const = 0;

            virtual void set_data(const void* data, u64 size, u64 offset) const = 0;

            virtual u64 get_size() const = 0;

            virtual BufferUsage get_usage() const = 0;

            virtual void* get_mapped_region() const = 0;
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

            virtual void wait(u64 timeout) const = 0;

            virtual void reset() const = 0;
    };

    class ITexture
    {
        public:
            virtual ~ITexture() = default;

            virtual void set_data(const void* data, u64 size) = 0;

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

            virtual ITexture* get_texture(u32 index) const = 0;

            virtual Result acquire_next_image(const ISemaphore* signal_semaphore, const IFence* fence) = 0;

            virtual void resize(const math::uvec2& extent) = 0;
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

            virtual void update(const IBuffer* buffer, u32 binding, u32 array_element, DescriptorType descriptor_type,
                                u64 offset) const = 0;

            virtual void update(const ITexture* texture, const ISampler* sampler, u32 binding, u32 array_element,
                                DescriptorType descriptor_type) const = 0;
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

            virtual void reset() const = 0;
    };

    class ICommandBuffer
    {
        public:
            virtual ~ICommandBuffer() = default;

            virtual void begin_recording() const = 0;

            virtual void end_recording() const = 0;

            virtual void reset() const = 0;

            virtual void set_viewport(const math::vec2& extent, const math::vec2& offset, f32 min_depth,
                                      f32 max_depth) const = 0;

            virtual void set_scissor(const math::uvec2& extent, const math::ivec2& offset) const = 0;

            virtual void begin_rendering(const IRenderPass* render_pass) const = 0;

            virtual void end_rendering() const = 0;

            virtual void bind_pipeline(const IGraphicsPipeline* pipeline) const = 0;

            virtual void bind_descriptor(const IGraphicsPipeline* pipeline, const IDescriptorSet* descriptor) const = 0;

            virtual void bind_vertex_buffers(u32 first_binding, u32 binding_count,
                                             const std::vector<const IBuffer*>& buffers,
                                             const std::vector<u64>& offsets) const = 0;

            virtual void bind_index_buffer(const IBuffer* buffer, u64 offset) const = 0;

            virtual void draw(u32 vertex_count, u32 instance_count, u32 first_vertex, u32 first_instance) const = 0;

            virtual void draw_indexed(u32 index_count, u32 instance_count, u32 first_index, i32 vertex_offset,
                                      u32 first_instance) const = 0;

            virtual void draw_indexed_indirect(const IBuffer* buffer, u64 offset, u32 draw_count, u32 stride) const = 0;

            virtual void pipeline_barrier(ITexture* texture, TextureLayout new_layout, AccessMask src_access_mask,
                                          AccessMask dst_access_mask, PipelineStage src_stage_mask,
                                          PipelineStage dst_stage_mask) const = 0;

            virtual void blit_texture(const ITexture* src_texture, const ITexture* dst_texture,
                                      Filter filter) const = 0;

            virtual void copy_texture(const ITexture* src_texture, const ITexture* dst_texture) const = 0;

            virtual void copy_buffer_to_texture(const IBuffer* buffer, const ITexture* texture) const = 0;
    };

    class IQueue
    {
        public:
            virtual ~IQueue() = default;

            virtual void submit(const ISemaphore* wait_semaphore, const ISemaphore* signal_semaphore,
                                const IFence* fence, const ICommandBuffer* command_buffer) const = 0;

            virtual Result present(const ISwapchain* swapchain, const ISemaphore* wait_semaphore) const = 0;
    };

    class IDevice
    {
        public:
            virtual ~IDevice() = default;

            virtual void wait_idle() const = 0;

            virtual void submit_commands_immediate(const std::function<void(ICommandBuffer& cmd)>& function) const = 0;

            virtual unique<ISemaphore> create_semaphore(const ISemaphoreDesc& desc) const = 0;

            virtual unique<IFence> create_fence(const IFenceDesc& desc) const = 0;

            virtual unique<ISwapchain> create_swapchain(const ISwapchainDesc& desc) const = 0;

            virtual unique<IQueue> create_queue(const IQueueDesc& desc) const = 0;

            virtual unique<IGraphicsPipeline> create_graphics_pipeline(const IGraphicsPipelineDesc& desc) const = 0;

            virtual unique<ICommandPool> create_command_pool(const ICommandPoolDesc& desc) const = 0;

            virtual unique<ICommandBuffer> create_command_buffer(const ICommandBufferDesc& desc) const = 0;

            virtual unique<IRenderingAttachment> create_render_attachment(
                const IRenderingAttachmentDesc& desc) const = 0;

            virtual unique<IRenderPass> create_render_pass(const IRenderPassDesc& desc) const = 0;

            virtual unique<ITexture> create_texture(const ITextureDesc& desc) = 0;

            virtual unique<IBuffer> create_buffer(const IBufferDesc& desc) const = 0;

            virtual unique<IDescriptorPool> create_descriptor_pool(const IDescriptorPoolDesc& desc) const = 0;

            virtual unique<IDescriptorSetLayout> create_descriptor_set_layout(
                const IDescriptorSetLayoutDesc& desc) const = 0;

            virtual unique<IDescriptorSet> create_descriptor_set(const IDescriptorSetDesc& desc) const = 0;

            virtual unique<ISampler> create_sampler(const ISamplerDesc& desc) const = 0;

            virtual DescriptorLimits get_descriptor_limits() const = 0;
    };

    unique<IDevice> create_device();
};  // namespace mag::gfx
