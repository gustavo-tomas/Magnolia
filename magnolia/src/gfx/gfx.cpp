#include "magnolia/gfx/gfx.hpp"

#include "backend/backend.hpp"
#include "magnolia/core/assert.hpp"
#include "magnolia/core/event.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/gfx/types.hpp"
#include "magnolia/math/functions.hpp"
#include "magnolia/platform/window.hpp"
#include "magnolia/resources/shader.hpp"

namespace mag::gfx
{
    struct BindingData
    {
            u32 binding = 0;
            u64 block_size = 0;
            u64 max_size_bytes = 0;
            DescriptorType descriptor_type = DescriptorType::Uniform;
            BufferHandle buffer_handle = Invalid_ID;
    };

    struct DescriptorData
    {
            DescriptorSetHandle descriptor_set = Invalid_ID;
            std::unordered_map<str, BindingData> bindings_map;
    };

    struct ShaderData
    {
            GraphicsPipelineHandle pipeline = Invalid_ID;
            DescriptorSetLayoutHandle descriptor_layout = Invalid_ID;
    };

    struct FrameData
    {
            CommandPoolHandle command_pool = Invalid_ID;
            CommandBufferHandle command_buffer = Invalid_ID;
            SemaphoreHandle available_semaphore = Invalid_ID;
            FenceHandle in_flight_fence = Invalid_ID;
            TextureHandle render_target_color = Invalid_ID;
            TextureHandle render_target_depth = Invalid_ID;
            DescriptorPoolHandle descriptor_pool = Invalid_ID;
            std::unordered_map<ShaderHandle, DescriptorData> descriptor_set_map;
    };

    struct GfxState
    {
            QueueHandle graphics_queue = Invalid_ID;
            QueueHandle present_queue = Invalid_ID;
            std::vector<FrameData> frames;
            std::vector<SemaphoreHandle> submit_semaphores;
            std::unordered_map<ShaderHandle, ShaderData> shaders;
            std::unordered_map<TextureHandle, SamplerHandle> textures;
            u32 current_frame = 0;
            ShaderHandle current_bound_shader = Invalid_ID;
    };

    // @TODO: hardcoded values
    static u64 get_descriptor_binding_count(const ShaderResourceBindingData& binding)
    {
        if (!binding.unbounded)
        {
            return binding.count;
        }

        const u64 combined_image_sampler_count = 1024;
        const u64 storage_count = 4ULL * 1024 * 1024;

        switch (binding.descriptor_type)
        {
            case ShaderResourceDescriptorType::CombinedImageSampler:
            {
                return combined_image_sampler_count;
            }

            case ShaderResourceDescriptorType::Storage:
            {
                return storage_count / binding.block_size_bytes;
            }

            default:
            {
                MAG_ASSERT(false, "Invalid descriptor type");
                return 0;
            }
        }
    }

    static GfxState* state = nullptr;

    b8 initialize(const GfxOptions& options)
    {
        state = new GfxState();
        create_device();

        // Swapchain
        // -------------------------------------------------------------------------------------------------
        ISwapchainDesc swapchain_desc = {};
        swapchain_desc.desired_present_mode = PresentMode::Mailbox;
        swapchain_desc.desired_extent = window::get_size();
        create_swapchain(swapchain_desc);

        // Queues
        // -------------------------------------------------------------------------------------------------
        state->graphics_queue = create_queue({.queue_type = QueueType::Graphics});
        state->present_queue = create_queue({.queue_type = QueueType::Present});

        // Command Pool, Command Buffers and Sync Objects
        // -------------------------------------------------------------------------------------------------
        const ISemaphoreDesc submit_semaphore_desc = {};

        state->submit_semaphores.resize(get_image_count_swapchain());

        for (SemaphoreHandle& submit_semaphore : state->submit_semaphores)
        {
            submit_semaphore = create_semaphore(submit_semaphore_desc);
        }

        // Triple buffering if the device supports it
        const u32 max_frames_in_flight = math::min(get_image_count_swapchain(), 3U);
        const math::uvec3 render_target_extent = math::uvec3(options.resolution, 1);
        const u32 max_descriptor_set_count = 1024;

        state->frames.resize(max_frames_in_flight);

        for (FrameData& frame : state->frames)
        {
            ICommandPoolDesc command_pool_desc = {};
            command_pool_desc.queue_type = QueueType::Graphics;
            frame.command_pool = create_command_pool(command_pool_desc);

            ICommandBufferDesc command_buffer_desc = {};
            command_buffer_desc.command_buffer_level = CommandBufferLevel::Primary;
            command_buffer_desc.command_pool = frame.command_pool;
            frame.command_buffer = create_command_buffer(command_buffer_desc);

            IFenceDesc fence_desc = {};
            fence_desc.signaled = true;

            const ISemaphoreDesc sem_desc = {};

            frame.available_semaphore = create_semaphore(sem_desc);
            frame.in_flight_fence = create_fence(fence_desc);

            ITextureDesc render_target_color_texture_desc = {};
            render_target_color_texture_desc.extent = render_target_extent;
            render_target_color_texture_desc.usage = TextureUsage::ColorAttachment | TextureUsage::TransferSrc;
            frame.render_target_color = create_texture(render_target_color_texture_desc);

            ITextureDesc render_target_depth_texture_desc = {};
            render_target_depth_texture_desc.extent = render_target_extent;
            render_target_depth_texture_desc.usage = TextureUsage::DepthStencilAttachment;
            render_target_depth_texture_desc.aspect = TextureAspect::Depth | TextureAspect::Stencil;
            render_target_depth_texture_desc.format = Format::D24_UNORM_S8_UINT;
            frame.render_target_depth = create_texture(render_target_depth_texture_desc);

            IDescriptorPoolDesc descriptor_pool_desc = {};
            descriptor_pool_desc.max_sets = max_descriptor_set_count;

            const DescriptorLimits limits = get_descriptor_limits();

            IDescriptorPoolSizeDesc size_desc_uniform = {};
            size_desc_uniform.type = DescriptorType::Uniform;
            size_desc_uniform.count = limits.max_per_stage_uniform_buffers * 4;

            IDescriptorPoolSizeDesc size_desc_storage = {};
            size_desc_storage.type = DescriptorType::Storage;
            size_desc_storage.count = limits.max_per_stage_storage_buffers * 4;

            IDescriptorPoolSizeDesc size_desc_combined_sampler = {};
            size_desc_combined_sampler.type = DescriptorType::CombinedImageSampler;
            size_desc_combined_sampler.count = limits.max_per_stage_combined_image_samplers * 4;

            descriptor_pool_desc.size_descs.push_back(size_desc_uniform);
            descriptor_pool_desc.size_descs.push_back(size_desc_storage);
            descriptor_pool_desc.size_descs.push_back(size_desc_combined_sampler);

            frame.descriptor_pool = create_descriptor_pool(descriptor_pool_desc);
        }

        return state != nullptr;
    }

    void shutdown()
    {
        wait_idle();

        delete state;
    }

    b8 begin_frame()
    {
        FrameData& current_frame = state->frames[state->current_frame];
        const TextureHandle render_target_color = current_frame.render_target_color;
        const TextureHandle render_target_depth = current_frame.render_target_depth;

        wait_fence(current_frame.in_flight_fence, Timeout);

        const Result result = acquire_next_image_swapchain(current_frame.available_semaphore, Invalid_ID);

        if (result == Result::ErrorOutOfDate || result == Result::SubOptimal)
        {
            resize_swapchain(window::get_size());
            return false;
        }

        if (result != Result::Success)
        {
            MAG_ASSERT(false, "Failed to acquire swapchain image");
            return false;
        }

        const math::uvec2 extent = math::uvec2(get_extent_texture(render_target_color));
        const math::vec4 clear_color = {0.4F, 0.6F, 0.8F, 1.0F};

        // Render Passes
        // -------------------------------------------------------------------------------------------------
        IRenderingAttachmentDesc color_attachment_desc = {};
        color_attachment_desc.type = RenderingAttachmentType::Color;
        color_attachment_desc.clear_color = clear_color;
        color_attachment_desc.texture = render_target_color;
        const RenderingAttachmentHandle color_attachment = create_render_attachment(color_attachment_desc);

        IRenderingAttachmentDesc depth_attachment_desc = {};
        depth_attachment_desc.type = RenderingAttachmentType::Depth;
        depth_attachment_desc.clear_depth = 1.0F;
        depth_attachment_desc.texture = render_target_depth;
        const RenderingAttachmentHandle depth_attachment = create_render_attachment(depth_attachment_desc);

        IRenderPassDesc render_pass_desc = {};
        render_pass_desc.extent = extent;
        render_pass_desc.color_attachments.push_back(color_attachment);
        render_pass_desc.depth_attachment = depth_attachment;
        const RenderPassHandle render_pass = create_render_pass(render_pass_desc);

        begin_recording_command_buffer(current_frame.command_buffer);

        static u32 f = 0;
        if (f < state->frames.size())
        {
            // Transition depth render target to optimal (only needs to be done once per frame)
            pipeline_barrier_command_buffer(
                current_frame.command_buffer, render_target_depth, TextureLayout::DepthStencilAttachment,
                AccessMask::None, AccessMask::DepthStencilAttachmentRead | AccessMask::DepthStencilAttachmentWrite,
                PipelineStage::TopOfPipe, PipelineStage::EarlyFragmentTest);

            f++;
        }

        // Prepare render target for rendering
        pipeline_barrier_command_buffer(
            current_frame.command_buffer, render_target_color, TextureLayout::ColorAttachment, AccessMask::None,
            AccessMask::ColorAttachmentWrite, PipelineStage::TopOfPipe, PipelineStage::ColorAttachmentOutput);

        // Flip the viewport to correct vulkan coordinate system
        const math::vec2 viewport_offset = math::vec2(0.0F, extent.y);
        auto viewport_extent = math::vec2(extent);
        viewport_extent.y = -viewport_extent.y;

        set_viewport_command_buffer(current_frame.command_buffer, viewport_extent, viewport_offset, 0.0F, 1.0F);
        set_scissor_command_buffer(current_frame.command_buffer, extent, {0.0F, 0.0F});
        begin_rendering_command_buffer(current_frame.command_buffer, render_pass);

        // RenderPass is a temporary resource, release it
        destroy_render_pass(render_pass);

        return true;
    }

    b8 end_frame()
    {
        u32& current_frame_idx = state->current_frame;
        FrameData& current_frame = state->frames[current_frame_idx];
        const TextureHandle render_target = current_frame.render_target_color;

        const u32 image_index = get_current_image_index_swapchain();
        const TextureHandle swapchain_texture = get_texture_swapchain(image_index);
        const SemaphoreHandle submit_semaphore = state->submit_semaphores[image_index];

        end_rendering_command_buffer(current_frame.command_buffer);

        // Transition render target to transfer
        pipeline_barrier_command_buffer(current_frame.command_buffer, render_target, TextureLayout::TransferSrc,
                                        AccessMask::ColorAttachmentWrite, AccessMask::TransferRead,
                                        PipelineStage::ColorAttachmentOutput, PipelineStage::Transfer);

        // Transition swapchain image to transfer
        pipeline_barrier_command_buffer(current_frame.command_buffer, swapchain_texture, TextureLayout::TransferDst,
                                        AccessMask::None, AccessMask::TransferWrite,
                                        PipelineStage::ColorAttachmentOutput, PipelineStage::Transfer);

        // Copy from the render target to the swapchain image
        blit_texture_command_buffer(current_frame.command_buffer, render_target, swapchain_texture, Filter::Linear);

        // Transition swapchain image to present
        pipeline_barrier_command_buffer(current_frame.command_buffer, swapchain_texture, TextureLayout::Present,
                                        AccessMask::TransferWrite, AccessMask::None, PipelineStage::Transfer,
                                        PipelineStage::BottomOfPipe);

        end_recording_command_buffer(current_frame.command_buffer);

        submit_queue(state->graphics_queue, current_frame.available_semaphore, submit_semaphore,
                     current_frame.in_flight_fence, current_frame.command_buffer);

        const Result result = present_queue(state->present_queue, submit_semaphore);

        if (result == Result::ErrorOutOfDate || result == Result::SubOptimal)
        {
            resize_swapchain(window::get_size());
            return false;
        }

        if (result != Result::Success)
        {
            MAG_ASSERT(false, "Failed to present swapchain image");
            return false;
        }

        current_frame_idx = (current_frame_idx + 1) % state->frames.size();

        return true;
    }

    static constexpr gfx::ShaderStage convert_resource_shader_stage(const ShaderResourceStage stage)
    {
        switch (stage)
        {
            case ShaderResourceStage::Vertex:
                return gfx::ShaderStage::Vertex;

            case ShaderResourceStage::Fragment:
                return gfx::ShaderStage::Fragment;
        }

        MAG_ASSERT(false, "Invalid shader resource stage");
        return gfx::ShaderStage::Vertex;
    }

    static constexpr gfx::PrimitiveTopology convert_topology(const ShaderResourceTopology topology)
    {
        switch (topology)
        {
            case ShaderResourceTopology::TriangleList:
                return gfx::PrimitiveTopology::TriangleList;

            case ShaderResourceTopology::TriangleStrip:
                return gfx::PrimitiveTopology::TriangleStrip;

            case ShaderResourceTopology::LineList:
                return gfx::PrimitiveTopology::LineList;
        }

        MAG_ASSERT(false, "Invalid shader resource topology");
        return gfx::PrimitiveTopology::TriangleList;
    }

    static constexpr gfx::DescriptorType convert_descriptor_type(const ShaderResourceDescriptorType descriptor_type)
    {
        switch (descriptor_type)
        {
            case ShaderResourceDescriptorType::Uniform:
                return gfx::DescriptorType::Uniform;

            case ShaderResourceDescriptorType::Storage:
                return gfx::DescriptorType::Storage;

            case ShaderResourceDescriptorType::CombinedImageSampler:
                return gfx::DescriptorType::CombinedImageSampler;
        }

        MAG_ASSERT(false, "Invalid shader resource descriptor type");
        return gfx::DescriptorType::Uniform;
    }

    static constexpr gfx::Format convert_resource_format(const ShaderResourceFormat format)
    {
        switch (format)
        {
            case ShaderResourceFormat::Undefined:
                return gfx::Format::Undefined;

            case ShaderResourceFormat::R32_UINT:
                return gfx::Format::R32_UINT;

            case ShaderResourceFormat::R32_SFLOAT:
                return gfx::Format::R32_SFLOAT;

            case ShaderResourceFormat::R32G32_SFLOAT:
                return gfx::Format::R32G32_SFLOAT;

            case ShaderResourceFormat::R32G32B32_SFLOAT:
                return gfx::Format::R32G32B32_SFLOAT;

            case ShaderResourceFormat::R32G32B32A32_SFLOAT:
                return gfx::Format::R32G32B32A32_SFLOAT;
        }

        MAG_ASSERT(false, "Invalid shader resource format");
        return gfx::Format::B8G8R8A8_SRGB;
    }

    static constexpr gfx::BlendOp convert_blend_op(const ShaderResourceBlendOp blend_op)
    {
        switch (blend_op)
        {
            case ShaderResourceBlendOp::Add:
                return gfx::BlendOp::Add;
        }

        MAG_ASSERT(false, "Invalid shader resource blend op");
        return gfx::BlendOp::Add;
    }

    static constexpr gfx::BlendFactor convert_blend_factor(const ShaderResourceBlendFactor blend_factor)
    {
        switch (blend_factor)
        {
            case ShaderResourceBlendFactor::One:
                return gfx::BlendFactor::One;

            case ShaderResourceBlendFactor::SrcAlpha:
                return gfx::BlendFactor::SrcAlpha;

            case ShaderResourceBlendFactor::OneMinusSrcAlpha:
                return gfx::BlendFactor::OneMinusSrcAlpha;
        }

        MAG_ASSERT(false, "Invalid shader resource blend factor");
        return gfx::BlendFactor::One;
    }

    static u32 create_handle()
    {
        static u32 handle_counter = 0;

        return handle_counter++;
    }

    static void set_buffer_data(BufferHandle buffer_handle, const void* data, u64 size, u64 offset = 0);

    static void set_texture_data(TextureHandle texture_handle, u64 size, const void* data);

    static BufferHandle create_buffer(const u64 size, const void* data, const BufferUsage usage)
    {
        IBufferDesc buffer_desc = {};
        buffer_desc.size_bytes = size;
        buffer_desc.buffer_usage = usage;
        buffer_desc.memory_usage = MemoryUsage::Auto;

        const BufferHandle handle = create_buffer(buffer_desc);

        if (data != nullptr)
        {
            set_data_buffer(handle, data, size, 0);
        }

        return handle;
    }

    static void destroy_buffer(const BufferHandle handle)
    {
        // @TODO: we need to make sure that a buffer is not in use when we delete it. A more robust approach would
        // be adding the buffer to a deletion queue and/or finding a way to query if the buffer is in use or not
        // before deleting. WaitIdle is, however, simpler.

        wait_idle();
        destroy_buffer_shitty_name(handle);
    }

    VertexBufferHandle create_vertex_buffer(const u64 size, const void* data)
    {
        return create_buffer(size, data, BufferUsage::Vertex);
    }

    void destroy_vertex_buffer(const VertexBufferHandle vertex_buffer_handle) { destroy_buffer(vertex_buffer_handle); }

    IndexBufferHandle create_index_buffer(const u64 size, const void* data)
    {
        return create_buffer(size, data, BufferUsage::Index);
    }

    void set_buffer_data(const BufferHandle buffer_handle, const void* data, const u64 size, const u64 offset)
    {
        set_data_buffer(buffer_handle, data, size, offset);
    }

    void set_uniform(const str& uniform_name, const void* data, const u32 array_element)
    {
        FrameData& current_frame = state->frames[state->current_frame];

        DescriptorData& descriptor_data = current_frame.descriptor_set_map[state->current_bound_shader];

        std::unordered_map<str, BindingData>& bindings_map = descriptor_data.bindings_map;

        const BindingData& binding = bindings_map[uniform_name];

        const BufferHandle buffer_handle = binding.buffer_handle;

        // Set the buffer data

        set_buffer_data(buffer_handle, data, binding.block_size, binding.block_size * array_element);

        // If we change the buffer, we need to update the descriptor sets (for each frame)

        update_descriptor_set(descriptor_data.descriptor_set, buffer_handle, binding.binding, array_element,
                              binding.descriptor_type, 0);
    }

    void set_uniform(const str& uniform_name, const TextureHandle texture_handle, const u32 array_element)
    {
        const FrameData& current_frame = state->frames[state->current_frame];

        const DescriptorData& descriptor_data = current_frame.descriptor_set_map.at(state->current_bound_shader);

        const std::unordered_map<str, BindingData>& bindings_map = descriptor_data.bindings_map;

        const BindingData& binding = bindings_map.at(uniform_name);

        const SamplerHandle sampler = state->textures[texture_handle];

        // If we change the texture, we need to update the descriptor sets (for each frame)

        update_descriptor_set(descriptor_data.descriptor_set, texture_handle, sampler, binding.binding, array_element,
                              binding.descriptor_type);
    }

    void set_uniform_static(const str& uniform_name, const void* data, const u32 array_element)
    {
        for (FrameData& frame : state->frames)
        {
            DescriptorData& descriptor_data = frame.descriptor_set_map[state->current_bound_shader];

            std::unordered_map<str, BindingData>& bindings_map = descriptor_data.bindings_map;

            const BindingData& binding = bindings_map[uniform_name];

            const BufferHandle buffer_handle = binding.buffer_handle;

            // Set the buffer data

            set_buffer_data(buffer_handle, data, binding.block_size, binding.block_size * array_element);

            // If we change the buffer, we need to update the descriptor sets (for each frame)

            update_descriptor_set(descriptor_data.descriptor_set, buffer_handle, binding.binding, array_element,
                                  binding.descriptor_type, 0);
        }
    }

    void set_uniform_static(const str& uniform_name, const TextureHandle texture_handle, const u32 array_element)
    {
        for (FrameData& frame : state->frames)
        {
            const DescriptorData& descriptor_data = frame.descriptor_set_map.at(state->current_bound_shader);

            const std::unordered_map<str, BindingData>& bindings_map = descriptor_data.bindings_map;

            const BindingData& binding = bindings_map.at(uniform_name);

            const SamplerHandle sampler = state->textures[texture_handle];

            // If we change the texture, we need to update the descriptor sets (for each frame)

            update_descriptor_set(descriptor_data.descriptor_set, texture_handle, sampler, binding.binding,
                                  array_element, binding.descriptor_type);
        }
    }

    TextureHandle create_texture(const u32 width, const u32 height, const u64 size, const void* pixels,
                                 const Format format)
    {
        ITextureDesc texture_desc = {};
        texture_desc.extent = math::uvec3(width, height, 1);
        texture_desc.usage = TextureUsage::ColorAttachment | TextureUsage::Sampled | TextureUsage::TransferDst;
        texture_desc.format = format;

        ISamplerDesc sampler_desc = {};
        sampler_desc.min_lod = 0.0F;
        sampler_desc.max_lod = static_cast<f32>(texture_desc.mip_levels);

        const TextureHandle handle = create_texture(texture_desc);

        state->textures[handle] = create_sampler(sampler_desc);

        if ((size > 0) && (pixels != nullptr))
        {
            set_texture_data(handle, size, pixels);
        }

        return handle;
    }

    void set_texture_data(const TextureHandle texture_handle, const u64 size, const void* data)
    {
        set_data_texture(texture_handle, data, size);
    }

    ShaderHandle create_shader(const ShaderResource& shader)
    {
        // Descriptors
        // -------------------------------------------------------------------------------------------------

        const ShaderHandle handle = create_handle();
        ShaderData& shader_data = state->shaders[handle];

        // Descriptor set layout
        IDescriptorSetLayoutDesc descriptor_layout_desc = {};

        u32 max_variable_descriptor_count = 1;
        std::unordered_map<str, BindingData> bindings_map;

        // @TODO: this is hardcoded to make my life easier. this is assuming that a descriptor will be used both in
        // the vertex and fragment shaders stages.
        for (const ShaderResourceDescriptorData& descriptor : shader.stages.at(ShaderResourceStage::Vertex).descriptors)
        {
            for (const ShaderResourceBindingData& binding : descriptor.bindings)
            {
                const u32 binding_count = get_descriptor_binding_count(binding);

                IDescriptorSetLayoutBindingDesc binding_desc = {};
                binding_desc.binding = binding.binding;
                binding_desc.descriptor_count = binding_count;
                binding_desc.variable_descriptor_count = binding.variable_count;
                binding_desc.descriptor_type = convert_descriptor_type(binding.descriptor_type);

                // @TODO: this is hardcoded to make my life easier
                binding_desc.stages = ShaderStage::Vertex | ShaderStage::Fragment;

                descriptor_layout_desc.binding_descs.push_back(binding_desc);

                BindingData binding_data = {};
                binding_data.binding = binding.binding;
                binding_data.block_size = binding.block_size_bytes;
                binding_data.max_size_bytes = binding_count * binding.block_size_bytes;
                binding_data.descriptor_type = convert_descriptor_type(binding.descriptor_type);

                if (binding.variable_count)
                {
                    max_variable_descriptor_count = math::max(max_variable_descriptor_count, binding_count);
                }

                bindings_map[binding.name] = binding_data;
            }
        }

        shader_data.descriptor_layout = create_descriptor_set_layout(descriptor_layout_desc);

        const DescriptorSetLayoutHandle descriptor_layout = shader_data.descriptor_layout;

        // Descriptor set

        for (FrameData& frame : state->frames)
        {
            IDescriptorSetDesc descriptor_desc = {};
            descriptor_desc.descriptor_layout = descriptor_layout;
            descriptor_desc.descriptor_pool = frame.descriptor_pool;
            descriptor_desc.max_variable_descriptor_count = max_variable_descriptor_count;

            frame.descriptor_set_map[handle].descriptor_set = create_descriptor_set(descriptor_desc);

            // Allocate memory for buffer uniforms

            for (auto& [binding_name, binding_data] : bindings_map)
            {
                if (binding_data.descriptor_type == DescriptorType::Uniform)
                {
                    binding_data.buffer_handle =
                        create_buffer(binding_data.max_size_bytes, nullptr, BufferUsage::Uniform);
                }

                else if (binding_data.descriptor_type == DescriptorType::Storage)
                {
                    binding_data.buffer_handle =
                        create_buffer(binding_data.max_size_bytes, nullptr, BufferUsage::Storage);
                }
            }

            // @TODO: this causes a bit of unecessary data duplication but its good enough for now
            frame.descriptor_set_map[handle].bindings_map = bindings_map;
        }

        // Graphics Pipeline
        // -------------------------------------------------------------------------------------------------
        const FrameData& current_frame = state->frames[state->current_frame];
        const Format color_format = get_format_texture(current_frame.render_target_color);
        const Format depth_format = get_format_texture(current_frame.render_target_depth);
        const math::uvec2 extent = math::uvec2(get_extent_texture(current_frame.render_target_color));

        IGraphicsPipelineDesc graphics_pipeline_desc = {};
        graphics_pipeline_desc.primitive_topology = convert_topology(shader.topology);
        graphics_pipeline_desc.color_attachment_format = color_format;
        graphics_pipeline_desc.depth_attachment_format = depth_format;
        graphics_pipeline_desc.extent = extent;
        graphics_pipeline_desc.descriptor_layouts.push_back(descriptor_layout);

        graphics_pipeline_desc.color_blend.blend_enable = shader.color_blend.blend_enable;
        graphics_pipeline_desc.color_blend.color_blend_op = convert_blend_op(shader.color_blend.color_blend_op);
        graphics_pipeline_desc.color_blend.alpha_blend_op = convert_blend_op(shader.color_blend.alpha_blend_op);
        graphics_pipeline_desc.color_blend.src_color_blend_factor =
            convert_blend_factor(shader.color_blend.src_color_blend_factor);
        graphics_pipeline_desc.color_blend.dst_color_blend_factor =
            convert_blend_factor(shader.color_blend.dst_color_blend_factor);
        graphics_pipeline_desc.color_blend.src_alpha_blend_factor =
            convert_blend_factor(shader.color_blend.src_alpha_blend_factor);
        graphics_pipeline_desc.color_blend.dst_alpha_blend_factor =
            convert_blend_factor(shader.color_blend.dst_alpha_blend_factor);

        u32 stride = 0;
        for (const ShaderResourceVertexInputData& vertex_input : shader.vertex_inputs)
        {
            IVertexAttributeDesc vertex_attribute_desc = {};
            vertex_attribute_desc.binding = 0;
            vertex_attribute_desc.format = convert_resource_format(vertex_input.format);
            vertex_attribute_desc.location = vertex_input.location;
            vertex_attribute_desc.offset = vertex_input.offset;

            graphics_pipeline_desc.vertex_attribute_descs.push_back(vertex_attribute_desc);

            stride += vertex_input.size;
        }

        if (!shader.vertex_inputs.empty())
        {
            IVertexBindingDesc vertex_binding_desc = {};
            vertex_binding_desc.binding = 0;
            vertex_binding_desc.input_rate = VertexInputRate::Vertex;
            vertex_binding_desc.stride = stride;

            graphics_pipeline_desc.vertex_binding_descs.push_back(vertex_binding_desc);
        }

        for (const auto& [shader_stage, shader_resource_data] : shader.stages)
        {
            IShaderModuleDesc shader_module_desc = {};
            shader_module_desc.code = shader_resource_data.code;
            shader_module_desc.stage = convert_resource_shader_stage(shader_stage);

            graphics_pipeline_desc.shader_modules.push_back(shader_module_desc);
        }

        shader_data.pipeline = create_graphics_pipeline(graphics_pipeline_desc);

        return handle;
    }

    void destroy_shader(const ShaderHandle shader_handle)
    {
        wait_idle();

        for (FrameData& frame : state->frames)
        {
            // Destroy uniform buffers
            for (const auto& [uniform_name, binding_data] : frame.descriptor_set_map[shader_handle].bindings_map)
            {
                destroy_buffer(binding_data.buffer_handle);
            }

            frame.descriptor_set_map.erase(shader_handle);
        }

        // Destroy graphics pipeline and descriptor set layouts
        state->shaders.erase(shader_handle);
    }

    void use_shader(const ShaderHandle& handle)
    {
        const FrameData& current_frame = state->frames[state->current_frame];
        const ShaderData& shader = state->shaders[handle];
        const DescriptorData& descriptor_data = current_frame.descriptor_set_map.at(handle);

        // Because we are using the descriptor indexing extension with the update after bind feature, we can bind
        // descriptors sets only once and later update them

        // Bind pipeline
        bind_pipeline_command_buffer(current_frame.command_buffer, shader.pipeline);

        // Bind the descriptor sets
        bind_descriptor_command_buffer(current_frame.command_buffer, shader.pipeline, descriptor_data.descriptor_set);

        state->current_bound_shader = handle;
    }

    void bind_vertex_buffer(const BufferHandle vertex_buffer_handle)
    {
        const FrameData& current_frame = state->frames[state->current_frame];

        bind_vertex_buffers_command_buffer(current_frame.command_buffer, 0, 1, {vertex_buffer_handle}, {0});
    }

    void bind_index_buffer(const BufferHandle index_buffer_handle)
    {
        const FrameData& current_frame = state->frames[state->current_frame];

        bind_index_buffer_command_buffer(current_frame.command_buffer, index_buffer_handle, 0);
    }

    void draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex, const u32 first_instance)
    {
        const FrameData& current_frame = state->frames[state->current_frame];

        draw_command_buffer(current_frame.command_buffer, vertex_count, instance_count, first_vertex, first_instance);
    }

    void draw_indexed(const u32 index_count, const u32 instance_count, const u32 first_index, const i32 vertex_offset,
                      const u32 first_instance)
    {
        const FrameData& current_frame = state->frames[state->current_frame];

        draw_indexed_command_buffer(current_frame.command_buffer, index_count, instance_count, first_index,
                                    vertex_offset, first_instance);
    }

    static void on_resize(const WindowResizeEvent& e)
    {
        wait_idle();

        resize_swapchain({e.width, e.height});
    }

    void on_event(const Event& e) { mag::dispatch_event<WindowResizeEvent>(e, on_resize); }
};  // namespace mag::gfx
