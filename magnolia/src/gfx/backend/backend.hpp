#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/gfx/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag::gfx
{
    using BufferHandle = u32;
    using FenceHandle = u32;
    using CommandBufferHandle = u32;
    using CommandPoolHandle = u32;
    using DescriptorPoolHandle = u32;
    using DescriptorSetHandle = u32;
    using DescriptorSetLayoutHandle = u32;
    using GraphicsPipelineHandle = u32;
    using QueueHandle = u32;
    using RenderingAttachmentHandle = u32;
    using RenderPassHandle = u32;
    using SamplerHandle = u32;
    using SemaphoreHandle = u32;
    using TextureHandle = u32;

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
            std::vector<DescriptorSetLayoutHandle> descriptor_layouts;
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
            CommandPoolHandle command_pool = 0;
    };

    struct IRenderingAttachmentDesc
    {
            math::vec4 clear_color = {1.0F, 1.0F, 1.0F, 1.0F};
            f32 clear_depth = 0.0F;
            u32 clear_stencil = 0;
            RenderingAttachmentType type = RenderingAttachmentType::Color;
            TextureHandle texture = 0;
    };

    struct IRenderPassDesc
    {
            math::uvec2 extent = {0, 0};
            math::ivec2 offset = {0, 0};
            std::vector<RenderingAttachmentHandle> color_attachments;
            RenderingAttachmentHandle depth_attachment = 0;
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
            DescriptorPoolHandle descriptor_pool = 0;
            DescriptorSetLayoutHandle descriptor_layout = 0;
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

    void create_device();

    SemaphoreHandle create_semaphore(const ISemaphoreDesc& desc);

    FenceHandle create_fence(const IFenceDesc& desc);

    void wait_fence(FenceHandle handle, u64 timeout);

    void create_swapchain(const ISwapchainDesc& desc);

    Result acquire_next_image_swapchain(SemaphoreHandle signal_semaphore_handle, FenceHandle fence_handle);

    void resize_swapchain(const math::uvec2& extent);

    TextureHandle get_texture_swapchain(u32 index);

    u32 get_image_count_swapchain();

    u32 get_current_image_index_swapchain();

    QueueHandle create_queue(const IQueueDesc& desc);

    void submit_queue(QueueHandle handle, SemaphoreHandle wait_semaphore_handle,
                      SemaphoreHandle signal_semaphore_handle, FenceHandle fence_handle,
                      CommandBufferHandle command_buffer_handle);

    Result present_queue(QueueHandle handle, SemaphoreHandle wait_semaphore_handle);

    GraphicsPipelineHandle create_graphics_pipeline(const IGraphicsPipelineDesc& desc);

    CommandPoolHandle create_command_pool(const ICommandPoolDesc& desc);

    CommandBufferHandle create_command_buffer(const ICommandBufferDesc& desc);

    void begin_recording_command_buffer(CommandBufferHandle handle);

    void end_recording_command_buffer(CommandBufferHandle handle);

    void pipeline_barrier_command_buffer(CommandBufferHandle handle, TextureHandle texture_handle,
                                         TextureLayout new_layout, AccessMask src_access_mask,
                                         AccessMask dst_access_mask, PipelineStage src_stage_mask,
                                         PipelineStage dst_stage_mask);

    void bind_pipeline_command_buffer(CommandBufferHandle handle, GraphicsPipelineHandle pipeline_handle);

    void bind_descriptor_command_buffer(CommandBufferHandle handle, GraphicsPipelineHandle pipeline_handle,
                                        DescriptorSetHandle descriptor_handle);

    void set_viewport_command_buffer(CommandBufferHandle handle, const math::vec2& extent, const math::vec2& offset,
                                     f32 min_depth, f32 max_depth);

    void set_scissor_command_buffer(CommandBufferHandle handle, const math::uvec2& extent, const math::ivec2& offset);

    void begin_rendering_command_buffer(CommandBufferHandle handle, RenderPassHandle render_pass_handle);

    void end_rendering_command_buffer(CommandBufferHandle handle);

    void bind_vertex_buffers_command_buffer(CommandBufferHandle handle, u32 first_binding, u32 binding_count,
                                            const std::vector<BufferHandle>& buffers, const std::vector<u64>& offsets);

    void bind_index_buffer_command_buffer(CommandBufferHandle handle, BufferHandle buffer_handle, u64 offset);

    void draw_command_buffer(CommandBufferHandle handle, u32 vertex_count, u32 instance_count, u32 first_vertex,
                             u32 first_instance);

    void draw_indexed_command_buffer(CommandBufferHandle handle, u32 index_count, u32 instance_count, u32 first_index,
                                     i32 vertex_offset, u32 first_instance);

    void blit_texture_command_buffer(CommandBufferHandle handle, TextureHandle src_texture_handle,
                                     TextureHandle dst_texture_handle, Filter filter);

    RenderingAttachmentHandle create_render_attachment(const IRenderingAttachmentDesc& desc);

    RenderPassHandle create_render_pass(const IRenderPassDesc& desc);

    void destroy_render_pass(RenderPassHandle handle);

    TextureHandle create_texture(const ITextureDesc& desc);

    void set_data_texture(TextureHandle handle, const void* data, u64 size);

    const math::uvec3& get_extent_texture(TextureHandle handle);

    Format get_format_texture(TextureHandle handle);

    BufferHandle create_buffer(const IBufferDesc& desc);

    void set_data_buffer(BufferHandle handle, const void* data, u64 data_size, u64 offset);

    DescriptorPoolHandle create_descriptor_pool(const IDescriptorPoolDesc& desc);

    DescriptorSetLayoutHandle create_descriptor_set_layout(const IDescriptorSetLayoutDesc& desc);

    DescriptorSetHandle create_descriptor_set(const IDescriptorSetDesc& desc);

    void update_descriptor_set(DescriptorSetHandle handle, BufferHandle buffer_handle, u32 binding, u32 array_element,
                               DescriptorType descriptor_type, u64 offset);

    void update_descriptor_set(DescriptorSetHandle handle, TextureHandle texture_handle, SamplerHandle sampler_handle,
                               u32 binding, u32 array_element, DescriptorType descriptor_type);

    SamplerHandle create_sampler(const ISamplerDesc& desc);

    void wait_idle();

    DescriptorLimits get_descriptor_limits();
};  // namespace mag::gfx
