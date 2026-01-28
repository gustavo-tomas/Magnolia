#pragma once

#include <functional>

#include "magnolia/core/types.hpp"
#include "magnolia/gfx/types.hpp"
#include "magnolia/math/types.hpp"

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
                math::uvec2 desired_extent = math::uvec2(256, 256);
        };

        struct IQueueDesc
        {
                QueueType queue_type;
        };

        struct IVertexAttributeDesc
        {
                Format format;
                u32 binding;
                u32 location;
                u32 offset;
        };

        struct IVertexBindingDesc
        {
                u32 binding;
                u32 stride;
                VertexInputRate input_rate;
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
                PrimitiveTopology primitive_topology;
                std::vector<IShaderModuleDesc> shader_modules;
                std::vector<const IDescriptorSetLayout*> descriptor_layouts;
                std::vector<IVertexAttributeDesc> vertex_attribute_descs;
                std::vector<IVertexBindingDesc> vertex_binding_descs;
                Format color_attachment_format;
                Format depth_attachment_format;
                math::uvec2 extent;
                IGraphicsPipelineColorBlend color_blend;
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
                b8 variable_descriptor_count;
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
                f32 min_lod = 0.0f;
                f32 max_lod = 0.0f;
                b8 anisotropy_enable = false;
                f32 max_anisotropy = 0.0f;
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

                virtual void set_data(const void* const data, const u64 size, const u64 offset = 0) const = 0;

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

                virtual void wait(const u64 timeout = Timeout) const = 0;

                virtual void reset() const = 0;
        };

        class ITexture
        {
            public:
                virtual ~ITexture() = default;

                virtual void set_data(const void* const data, const u64 size) = 0;

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

                virtual ITexture* get_texture(const u32 index) const = 0;

                virtual Result acquire_next_image(const ISemaphore* const signal_semaphore,
                                                  const IFence* const fence = nullptr) = 0;

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

                virtual void update(const IBuffer* const buffer, const u32 binding, const u32 array_element,
                                    const DescriptorType descriptor_type, const u64 offset = 0) const = 0;

                virtual void update(const ITexture* const texture, const ISampler* const sampler, const u32 binding,
                                    const u32 array_element, const DescriptorType descriptor_type) const = 0;
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

                virtual void set_viewport(const math::vec2& extent, const math::vec2& offset = {0.0f, 0.0f},
                                          const f32 min_depth = 0.0f, const f32 max_depth = 1.0f) const = 0;

                virtual void set_scissor(const math::uvec2& extent, const math::ivec2& offset = {0.0f, 0.0f}) const = 0;

                virtual void begin_rendering(const IRenderPass* const render_pass) const = 0;

                virtual void end_rendering() const = 0;

                virtual void bind_pipeline(const IGraphicsPipeline* const pipeline) const = 0;

                virtual void bind_descriptor(const IGraphicsPipeline* const pipeline,
                                             const IDescriptorSet* const descriptor) const = 0;

                virtual void bind_vertex_buffers(const u32 first_binding, const u32 binding_count,
                                                 const std::vector<const IBuffer*>& buffers,
                                                 const std::vector<u64>& offsets) const = 0;

                virtual void bind_index_buffer(const IBuffer* const buffer, const u64 offset) const = 0;

                virtual void draw(const u32 vertex_count, const u32 instance_count = 1, const u32 first_vertex = 0,
                                  const u32 first_instance = 0) const = 0;

                virtual void draw_indexed(const u32 index_count, const u32 instance_count = 1,
                                          const u32 first_index = 0, const i32 vertex_offset = 0,
                                          const u32 first_instance = 0) const = 0;

                virtual void pipeline_barrier(ITexture* const texture, const TextureLayout new_layout,
                                              const AccessMask src_access_mask, const AccessMask dst_access_mask,
                                              const PipelineStage src_stage_mask,
                                              const PipelineStage dst_stage_mask) const = 0;

                virtual void blit_texture(const ITexture* const src_texture, const ITexture* const dst_texture,
                                          const Filter filter) const = 0;

                virtual void copy_texture(const ITexture* const src_texture,
                                          const ITexture* const dst_texture) const = 0;

                virtual void copy_buffer_to_texture(const IBuffer* const buffer,
                                                    const ITexture* const texture) const = 0;
        };

        class IQueue
        {
            public:
                virtual ~IQueue() = default;

                virtual void submit(const ISemaphore* const wait_semaphore, const ISemaphore* const signal_semaphore,
                                    const IFence* const fence, const ICommandBuffer* const command_buffer) const = 0;

                virtual Result present(const ISwapchain* const swapchain,
                                       const ISemaphore* const wait_semaphore) const = 0;
        };

        class IDevice
        {
            public:
                virtual ~IDevice() = default;

                virtual void wait_idle() const = 0;

                virtual void submit_commands_immediate(std::function<void(ICommandBuffer& cmd)>&& function) const = 0;

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
        };

        unique<IDevice> create_device();
    };  // namespace gfx
};  // namespace mag
