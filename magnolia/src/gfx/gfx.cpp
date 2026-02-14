#include "magnolia/gfx/gfx.hpp"

#include "backend/backend.hpp"
#include "magnolia/core/assert.hpp"
#include "magnolia/core/event.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/gfx/types.hpp"
#include "magnolia/platform/window.hpp"
#include "magnolia/resources/shader.hpp"

namespace mag
{
    namespace gfx
    {
        using BufferHandle = u32;

        struct BindingData
        {
                u32 binding = 0;
                u64 block_size = 0;
                u64 max_size_bytes = 0;
                DescriptorType descriptor_type;
                BufferHandle buffer_handle = Invalid_ID;
        };

        struct DescriptorData
        {
                unique<IDescriptorSet> descriptor_set;
                std::unordered_map<str, BindingData> bindings_map;
                BufferHandle last_bound_buffer = Invalid_ID;
                TextureHandle last_bound_texture = Invalid_ID;
        };

        struct ShaderData
        {
                unique<IGraphicsPipeline> pipeline;
                unique<IDescriptorSetLayout> descriptor_layout;
        };

        struct TextureData
        {
                unique<ITexture> texture;
                unique<ISampler> sampler;
        };

        struct FrameData
        {
                unique<ICommandPool> command_pool;
                unique<ICommandBuffer> command_buffer;
                unique<ISemaphore> available_semaphore;
                unique<ISemaphore> finished_semaphore;
                unique<IFence> in_flight_fence;
                unique<ITexture> render_target_color;
                unique<ITexture> render_target_depth;
                unique<IDescriptorPool> descriptor_pool;
                std::unordered_map<ShaderHandle, DescriptorData> descriptor_set_map;
        };

        struct GfxState
        {
                unique<IDevice> device;
                unique<ISwapchain> swapchain;
                unique<IQueue> graphics_queue;
                unique<IQueue> present_queue;
                std::vector<FrameData> frames;
                std::unordered_map<ShaderHandle, ShaderData> shaders;
                std::unordered_map<BufferHandle, unique<IBuffer>> buffers;
                std::unordered_map<TextureHandle, TextureData> textures;
                u32 current_frame = 0;
                ShaderHandle current_bound_shader = Invalid_ID;
        };

        static GfxState* state = nullptr;

        b8 initialize(const GfxOptions& options)
        {
            state = new GfxState();
            state->device = create_device();

            // Swapchain
            // -------------------------------------------------------------------------------------------------
            ISwapchainDesc swapchain_desc = {};
            swapchain_desc.desired_present_mode = PresentMode::Mailbox;
            swapchain_desc.desired_extent = window::get_size();
            state->swapchain = state->device->create_swapchain(swapchain_desc);

            // Queues
            // -------------------------------------------------------------------------------------------------
            state->graphics_queue = state->device->create_queue({.queue_type = QueueType::Graphics});
            state->present_queue = state->device->create_queue({.queue_type = QueueType::Present});

            // Command Pool, Command Buffers and Sync Objects
            // -------------------------------------------------------------------------------------------------

            // Triple buffering if the device supports it
            const u32 max_frames_in_flight = math::min(state->swapchain->get_image_count(), 3u);
            const math::uvec3 render_target_extent = math::uvec3(options.resolution, 1);

            state->frames.resize(max_frames_in_flight);

            for (u32 i = 0; i < state->frames.size(); i++)
            {
                ICommandPoolDesc command_pool_desc = {};
                command_pool_desc.queue_type = QueueType::Graphics;
                state->frames[i].command_pool = state->device->create_command_pool(command_pool_desc);

                ICommandBufferDesc command_buffer_desc = {};
                command_buffer_desc.command_buffer_level = CommandBufferLevel::Primary;
                command_buffer_desc.command_pool = state->frames[i].command_pool.get();
                state->frames[i].command_buffer = state->device->create_command_buffer(command_buffer_desc);

                IFenceDesc fence_desc = {};
                fence_desc.signaled = true;

                ISemaphoreDesc sem_desc = {};

                state->frames[i].available_semaphore = state->device->create_semaphore(sem_desc);
                state->frames[i].finished_semaphore = state->device->create_semaphore(sem_desc);
                state->frames[i].in_flight_fence = state->device->create_fence(fence_desc);

                ITextureDesc render_target_color_texture_desc = {};
                render_target_color_texture_desc.extent = render_target_extent;
                render_target_color_texture_desc.usage = TextureUsage::ColorAttachment | TextureUsage::TransferSrc;
                state->frames[i].render_target_color = state->device->create_texture(render_target_color_texture_desc);

                ITextureDesc render_target_depth_texture_desc = {};
                render_target_depth_texture_desc.extent = render_target_extent;
                render_target_depth_texture_desc.usage = TextureUsage::DepthStencilAttachment;
                render_target_depth_texture_desc.aspect = TextureAspect::Depth;
                render_target_depth_texture_desc.format = Format::D24_UNORM_S8_UINT;
                state->frames[i].render_target_depth = state->device->create_texture(render_target_depth_texture_desc);

                IDescriptorPoolDesc descriptor_pool_desc = {};
                descriptor_pool_desc.max_sets = 1024;

                IDescriptorPoolSizeDesc size_desc_uniform = {};
                size_desc_uniform.type = DescriptorType::Uniform;
                size_desc_uniform.size = 64;

                IDescriptorPoolSizeDesc size_desc_storage = {};
                size_desc_storage.type = DescriptorType::Storage;
                size_desc_storage.size = 64;

                IDescriptorPoolSizeDesc size_desc_combined_sampler = {};
                size_desc_combined_sampler.type = DescriptorType::CombinedImageSampler;
                size_desc_combined_sampler.size = 64;

                descriptor_pool_desc.size_descs.push_back(size_desc_uniform);
                descriptor_pool_desc.size_descs.push_back(size_desc_storage);
                descriptor_pool_desc.size_descs.push_back(size_desc_combined_sampler);

                state->frames[i].descriptor_pool = state->device->create_descriptor_pool(descriptor_pool_desc);
            }

            return state->device != nullptr;
        }

        void shutdown()
        {
            state->device->wait_idle();

            delete state;
        }

        b8 begin_frame()
        {
            FrameData& current_frame = state->frames[state->current_frame];
            const unique<ITexture>& render_target_color = current_frame.render_target_color;
            const unique<ITexture>& render_target_depth = current_frame.render_target_depth;

            current_frame.in_flight_fence->wait();

            const Result result = state->swapchain->acquire_next_image(current_frame.available_semaphore.get());

            if (result == Result::ErrorOutOfDate || result == Result::SubOptimal)
            {
                state->swapchain->resize(window::get_size());
                return false;
            }

            if (result != Result::Success)
            {
                MAG_ASSERT(false, "Failed to acquire swapchain image");
                return false;
            }

            const math::uvec2 extent = render_target_color->get_extent();

            // Render Passes
            // -------------------------------------------------------------------------------------------------
            unique<IRenderPass> render_pass;
            unique<IRenderingAttachment> color_attachment;
            unique<IRenderingAttachment> depth_attachment;

            IRenderingAttachmentDesc color_attachment_desc = {};
            color_attachment_desc.type = RenderingAttachmentType::Color;
            color_attachment_desc.clear_color = {0.4f, 0.6f, 0.8f, 1.0f};
            color_attachment_desc.texture = render_target_color.get();
            color_attachment = state->device->create_render_attachment(color_attachment_desc);

            IRenderingAttachmentDesc depth_attachment_desc = {};
            depth_attachment_desc.type = RenderingAttachmentType::Depth;
            depth_attachment_desc.clear_depth = 1.0f;
            depth_attachment_desc.texture = render_target_depth.get();
            depth_attachment = state->device->create_render_attachment(depth_attachment_desc);

            IRenderPassDesc render_pass_desc = {};
            render_pass_desc.extent = extent;
            render_pass_desc.color_attachments.push_back(color_attachment.get());
            render_pass_desc.depth_attachment = depth_attachment.get();
            render_pass = state->device->create_render_pass(render_pass_desc);

            current_frame.command_buffer->begin_recording();

            // Prepare render target for rendering
            current_frame.command_buffer->pipeline_barrier(
                render_target_color.get(), TextureLayout::ColorAttachment, AccessMask::None,
                AccessMask::ColorAttachmentWrite, PipelineStage::TopOfPipe, PipelineStage::ColorAttachmentOutput);

            // Flip the viewport to correct vulkan coordinate system
            math::vec2 viewport_offset = math::vec2(0.0f, extent.y);
            math::vec2 viewport_extent = extent;
            viewport_extent.y = -viewport_extent.y;

            current_frame.command_buffer->set_viewport(viewport_extent, viewport_offset);
            current_frame.command_buffer->set_scissor(extent);

            current_frame.command_buffer->begin_rendering(render_pass.get());

            return true;
        }

        b8 end_frame()
        {
            u32& current_frame_idx = state->current_frame;
            FrameData& current_frame = state->frames[current_frame_idx];
            const unique<ITexture>& render_target = current_frame.render_target_color;

            const u32 image_index = state->swapchain->get_current_image_index();
            ITexture* const swapchain_texture = state->swapchain->get_texture(image_index);

            current_frame.command_buffer->end_rendering();

            // Transition render target to transfer
            current_frame.command_buffer->pipeline_barrier(
                render_target.get(), TextureLayout::TransferSrc, AccessMask::ColorAttachmentWrite,
                AccessMask::TransferRead, PipelineStage::ColorAttachmentOutput, PipelineStage::Transfer);

            // Transition swapchain image to transfer
            current_frame.command_buffer->pipeline_barrier(swapchain_texture, TextureLayout::TransferDst,
                                                           AccessMask::None, AccessMask::TransferWrite,
                                                           PipelineStage::TopOfPipe, PipelineStage::Transfer);

            // Copy from the render target to the swapchain image
            current_frame.command_buffer->blit_texture(render_target.get(), swapchain_texture, Filter::Linear);

            // Transition swapchain image to present
            current_frame.command_buffer->pipeline_barrier(swapchain_texture, TextureLayout::Present,
                                                           AccessMask::TransferWrite, AccessMask::MemoryRead,
                                                           PipelineStage::Transfer, PipelineStage::BottomOfPipe);

            current_frame.command_buffer->end_recording();

            state->graphics_queue->submit(current_frame.available_semaphore.get(),
                                          current_frame.finished_semaphore.get(), current_frame.in_flight_fence.get(),
                                          current_frame.command_buffer.get());

            const Result result =
                state->present_queue->present(state->swapchain.get(), current_frame.finished_semaphore.get());

            if (result == Result::ErrorOutOfDate || result == Result::SubOptimal)
            {
                state->swapchain->resize(window::get_size());
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

        static const std::unordered_map<ShaderResourceStage, gfx::ShaderStage> convert_resource_shader_stage = {
            {ShaderResourceStage::Vertex, gfx::ShaderStage::Vertex},
            {ShaderResourceStage::Fragment, gfx::ShaderStage::Fragment},
        };

        static const std::unordered_map<ShaderResourceTopology, gfx::PrimitiveTopology> convert_topology = {
            {ShaderResourceTopology::TriangleList, gfx::PrimitiveTopology::TriangleList},
            {ShaderResourceTopology::TriangleStrip, gfx::PrimitiveTopology::TriangleStrip},
            {ShaderResourceTopology::LineList, gfx::PrimitiveTopology::LineList},
        };

        static const std::unordered_map<ShaderResourceDescriptorType, gfx::DescriptorType> convert_descriptor_type = {
            {ShaderResourceDescriptorType::Uniform, gfx::DescriptorType::Uniform},
            {ShaderResourceDescriptorType::Storage, gfx::DescriptorType::Storage},
            {ShaderResourceDescriptorType::CombinedImageSampler, gfx::DescriptorType::CombinedImageSampler},
        };

        static const std::unordered_map<ShaderResourceFormat, gfx::Format> convert_resource_format = {
            {ShaderResourceFormat::Undefined, gfx::Format::Undefined},
            {ShaderResourceFormat::R32_UINT, gfx::Format::R32_UINT},
            {ShaderResourceFormat::R32_SFLOAT, gfx::Format::R32_SFLOAT},
            {ShaderResourceFormat::R32G32_SFLOAT, gfx::Format::R32G32_SFLOAT},
            {ShaderResourceFormat::R32G32B32_SFLOAT, gfx::Format::R32G32B32_SFLOAT},
            {ShaderResourceFormat::R32G32B32A32_SFLOAT, gfx::Format::R32G32B32A32_SFLOAT},
        };

        static const std::unordered_map<ShaderResourceBlendOp, gfx::BlendOp> convert_blend_op = {
            {ShaderResourceBlendOp::Add, gfx::BlendOp::Add},
        };

        static const std::unordered_map<ShaderResourceBlendFactor, gfx::BlendFactor> convert_blend_factor = {
            {ShaderResourceBlendFactor::One, gfx::BlendFactor::One},
            {ShaderResourceBlendFactor::SrcAlpha, gfx::BlendFactor::SrcAlpha},
            {ShaderResourceBlendFactor::OneMinusSrcAlpha, gfx::BlendFactor::OneMinusSrcAlpha},
        };

        static u32 create_handle()
        {
            // @TODO: this is a pretty simple way to create handles, but it works
            static u32 handle_counter = 0;

            return handle_counter++;
        }

        static void set_buffer_data(const BufferHandle buffer_handle, const void* data, const u64 size,
                                    const u64 offset = 0);

        static void set_texture_data(const TextureHandle texture_handle, const u64 size, const void* data);

        static BufferHandle create_buffer(const u64 size, const void* data, const BufferUsage usage)
        {
            const BufferHandle handle = create_handle();

            IBufferDesc buffer_desc = {};
            buffer_desc.size_bytes = size;
            buffer_desc.buffer_usage = usage;
            buffer_desc.memory_usage = MemoryUsage::Auto;

            state->buffers[handle] = state->device->create_buffer(buffer_desc);

            if (data != nullptr)
            {
                state->buffers[handle]->set_data(data, size);
            }

            return handle;
        }

        VertexBufferHandle create_vertex_buffer(const u64 size, const void* data)
        {
            return create_buffer(size, data, BufferUsage::Vertex);
        }

        void destroy_vertex_buffer(const VertexBufferHandle vertex_buffer_handle)
        {
            // @TODO: we need to make sure that a buffer is not in use when we delete it. A more robust approach would
            // be adding the buffer to a deletion queue and/or finding a way to query if the buffer is in use or not
            // before deleting. WaitIdle is, however, simpler.
            state->device->wait_idle();
            state->buffers.erase(vertex_buffer_handle);
        }

        IndexBufferHandle create_index_buffer(const u64 size, const void* data)
        {
            return create_buffer(size, data, BufferUsage::Index);
        }

        void set_buffer_data(const BufferHandle buffer_handle, const void* data, const u64 size, const u64 offset)
        {
            state->buffers[buffer_handle]->set_data(data, size, offset);
        }

        void set_uniform(const str& uniform_name, const void* data, const u32 array_element)
        {
            FrameData& current_frame = state->frames[state->current_frame];

            DescriptorData& descriptor_data = current_frame.descriptor_set_map[state->current_bound_shader];

            std::unordered_map<str, BindingData>& bindings_map = descriptor_data.bindings_map;

            BindingData& binding = bindings_map[uniform_name];

            const BufferHandle buffer_handle = binding.buffer_handle;
            const unique<IBuffer>& buffer = state->buffers[buffer_handle];

            // Set the buffer data

            set_buffer_data(buffer_handle, data, binding.block_size, binding.block_size * array_element);

            // If we change the buffer, we need to update the descriptor sets (for each frame)

            if (descriptor_data.last_bound_buffer != buffer_handle)
            {
                descriptor_data.descriptor_set->update(buffer.get(), binding.binding, array_element,
                                                       binding.descriptor_type);

                descriptor_data.last_bound_buffer = buffer_handle;
            }
        }

        void set_uniform_static(const str& uniform_name, const void* data, const u32 array_element)
        {
            for (u32 i = 0; i < state->frames.size(); i++)
            {
                FrameData& current_frame = state->frames[i];

                DescriptorData& descriptor_data = current_frame.descriptor_set_map[state->current_bound_shader];

                std::unordered_map<str, BindingData>& bindings_map = descriptor_data.bindings_map;

                BindingData& binding = bindings_map[uniform_name];

                const BufferHandle buffer_handle = binding.buffer_handle;
                const unique<IBuffer>& buffer = state->buffers[buffer_handle];

                // Set the buffer data

                set_buffer_data(buffer_handle, data, binding.block_size, binding.block_size * array_element);

                // If we change the buffer, we need to update the descriptor sets (for each frame)

                if (descriptor_data.last_bound_buffer != buffer_handle)
                {
                    descriptor_data.descriptor_set->update(buffer.get(), binding.binding, array_element,
                                                           binding.descriptor_type);

                    descriptor_data.last_bound_buffer = buffer_handle;
                }
            }
        }

        void set_uniform(const str& uniform_name, const TextureHandle texture_handle, const u32 array_element)
        {
            FrameData& current_frame = state->frames[state->current_frame];

            DescriptorData& descriptor_data = current_frame.descriptor_set_map[state->current_bound_shader];

            const std::unordered_map<str, BindingData>& bindings_map = descriptor_data.bindings_map;

            const BindingData& binding = bindings_map.at(uniform_name);

            const unique<ITexture>& texture = state->textures[texture_handle].texture;
            const unique<ISampler>& sampler = state->textures[texture_handle].sampler;

            // If we change the texture, we need to update the descriptor sets (for each frame)

            // @TODO: this is bugged
            // if (descriptor_data.last_bound_texture != texture_handle)
            {
                descriptor_data.descriptor_set->update(texture.get(), sampler.get(), binding.binding, array_element,
                                                       binding.descriptor_type);

                descriptor_data.last_bound_texture = texture_handle;
            }
        }

        TextureHandle create_texture(const u32 width, const u32 height, const u64 size, const void* pixels,
                                     const Format format)
        {
            const TextureHandle handle = create_handle();

            ITextureDesc texture_desc = {};
            texture_desc.extent = math::uvec3(width, height, 1);
            texture_desc.usage = TextureUsage::ColorAttachment | TextureUsage::Sampled | TextureUsage::TransferDst;
            texture_desc.format = format;

            ISamplerDesc sampler_desc = {};
            sampler_desc.min_lod = 0.0f;
            sampler_desc.max_lod = static_cast<f32>(texture_desc.mip_levels);

            state->textures[handle].sampler = state->device->create_sampler(sampler_desc);
            state->textures[handle].texture = state->device->create_texture(texture_desc);

            if ((size > 0) && (pixels != nullptr))
            {
                set_texture_data(handle, size, pixels);
            }

            return handle;
        }

        void set_texture_data(const TextureHandle texture_handle, const u64 size, const void* data)
        {
            state->textures[texture_handle].texture->set_data(data, size);
        }

        ShaderHandle create_shader(const ShaderResource& shader)
        {
            // Descriptors
            // -------------------------------------------------------------------------------------------------

            const ShaderHandle handle = create_handle();

            // Descriptor set layout
            IDescriptorSetLayoutDesc descriptor_layout_desc = {};

            u32 max_variable_descriptor_count = 1;
            std::unordered_map<str, BindingData> bindings_map;

            // @TODO: this is hardcoded to make my life easier. this is assuming that a descriptor will be used both in
            // the vertex and fragment shaders.
            for (const ShaderResourceDescriptorData& descriptor :
                 shader.stages.at(ShaderResourceStage::Vertex).descriptors)
            {
                for (const ShaderResourceBindingData& binding : descriptor.bindings)
                {
                    IDescriptorSetLayoutBindingDesc binding_desc = {};
                    binding_desc.binding = binding.binding;
                    binding_desc.descriptor_count = binding.count;
                    binding_desc.variable_descriptor_count = binding.variable_count;
                    binding_desc.descriptor_type = convert_descriptor_type.at(binding.descriptor_type);

                    // @TODO: this is hardcoded to make my life easier
                    binding_desc.stages = ShaderStage::Vertex | ShaderStage::Fragment;

                    descriptor_layout_desc.binding_descs.push_back(binding_desc);

                    BindingData binding_data = {};
                    binding_data.binding = binding.binding;
                    binding_data.block_size = binding.block_size_bytes;
                    binding_data.max_size_bytes = binding.count * binding.block_size_bytes;
                    binding_data.descriptor_type = convert_descriptor_type.at(binding.descriptor_type);

                    if (binding.variable_count)
                    {
                        max_variable_descriptor_count = math::max(max_variable_descriptor_count, binding.count);
                    }

                    bindings_map[binding.name] = binding_data;
                }
            }

            state->shaders[handle].descriptor_layout =
                state->device->create_descriptor_set_layout(descriptor_layout_desc);

            const unique<IDescriptorSetLayout>& descriptor_layout = state->shaders[handle].descriptor_layout;

            // Descriptor set

            for (u32 i = 0; i < state->frames.size(); i++)
            {
                IDescriptorSetDesc descriptor_desc = {};
                descriptor_desc.descriptor_layout = descriptor_layout.get();
                descriptor_desc.descriptor_pool = state->frames[i].descriptor_pool.get();
                descriptor_desc.max_variable_descriptor_count = max_variable_descriptor_count;

                state->frames[i].descriptor_set_map[handle].descriptor_set =
                    state->device->create_descriptor_set(descriptor_desc);

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
                state->frames[i].descriptor_set_map[handle].bindings_map = bindings_map;
            }

            // Graphics Pipeline
            // -------------------------------------------------------------------------------------------------
            const Format color_format = state->frames[state->current_frame].render_target_color->get_format();
            const Format depth_format = state->frames[state->current_frame].render_target_depth->get_format();
            const math::uvec2 extent = state->frames[state->current_frame].render_target_color->get_extent();

            IGraphicsPipelineDesc graphics_pipeline_desc = {};
            graphics_pipeline_desc.primitive_topology = convert_topology.at(shader.topology);
            graphics_pipeline_desc.color_attachment_format = color_format;
            graphics_pipeline_desc.depth_attachment_format = depth_format;
            graphics_pipeline_desc.extent = extent;
            graphics_pipeline_desc.descriptor_layouts.push_back(descriptor_layout.get());

            graphics_pipeline_desc.color_blend.blend_enable = shader.color_blend.blend_enable;
            graphics_pipeline_desc.color_blend.color_blend_op = convert_blend_op.at(shader.color_blend.color_blend_op);
            graphics_pipeline_desc.color_blend.alpha_blend_op = convert_blend_op.at(shader.color_blend.alpha_blend_op);
            graphics_pipeline_desc.color_blend.src_color_blend_factor =
                convert_blend_factor.at(shader.color_blend.src_color_blend_factor);
            graphics_pipeline_desc.color_blend.dst_color_blend_factor =
                convert_blend_factor.at(shader.color_blend.dst_color_blend_factor);
            graphics_pipeline_desc.color_blend.src_alpha_blend_factor =
                convert_blend_factor.at(shader.color_blend.src_alpha_blend_factor);
            graphics_pipeline_desc.color_blend.dst_alpha_blend_factor =
                convert_blend_factor.at(shader.color_blend.dst_alpha_blend_factor);

            u32 stride = 0;
            for (const ShaderResourceVertexInputData& vertex_input : shader.vertex_inputs)
            {
                IVertexAttributeDesc vertex_attribute_desc = {};
                vertex_attribute_desc.binding = 0;
                vertex_attribute_desc.format = convert_resource_format.at(vertex_input.format);
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

            for (const auto& [shader_stage, shader_data] : shader.stages)
            {
                IShaderModuleDesc shader_module_desc = {};
                shader_module_desc.code = shader_data.code;
                shader_module_desc.stage = convert_resource_shader_stage.at(shader_stage);

                graphics_pipeline_desc.shader_modules.push_back(shader_module_desc);
            }

            state->shaders[handle].pipeline = state->device->create_graphics_pipeline(graphics_pipeline_desc);

            return handle;
        }

        void destroy_shader(const ShaderHandle shader_handle)
        {
            state->device->wait_idle();
            state->shaders.erase(shader_handle);

            for (FrameData& frame : state->frames)
            {
                frame.descriptor_set_map.erase(shader_handle);
            }
        }

        void use_shader(const ShaderHandle& handle)
        {
            const FrameData& current_frame = state->frames[state->current_frame];
            const ShaderData& shader = state->shaders[handle];
            const DescriptorData& descriptor_data = current_frame.descriptor_set_map.at(handle);

            // Because we are using the descriptor indexing extension with the update after bind feature, we can bind
            // descriptors sets only once and later update them

            // Bind pipeline
            current_frame.command_buffer->bind_pipeline(state->shaders[handle].pipeline.get());

            // Bind the descriptor sets
            current_frame.command_buffer->bind_descriptor(shader.pipeline.get(), descriptor_data.descriptor_set.get());

            state->current_bound_shader = handle;
        }

        void bind_vertex_buffer(const BufferHandle vertex_buffer_handle)
        {
            const FrameData& current_frame = state->frames[state->current_frame];

            current_frame.command_buffer->bind_vertex_buffers(0, 1, {state->buffers[vertex_buffer_handle].get()}, {0});
        }

        void bind_index_buffer(const BufferHandle index_buffer_handle)
        {
            const FrameData& current_frame = state->frames[state->current_frame];

            current_frame.command_buffer->bind_index_buffer(state->buffers[index_buffer_handle].get(), 0);
        }

        void draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex, const u32 first_instance)
        {
            const FrameData& current_frame = state->frames[state->current_frame];

            current_frame.command_buffer->draw(vertex_count, instance_count, first_vertex, first_instance);
        }

        void draw_indexed(const u32 index_count, const u32 instance_count, const u32 first_index,
                          const i32 vertex_offset, const u32 first_instance)
        {
            const FrameData& current_frame = state->frames[state->current_frame];

            current_frame.command_buffer->draw_indexed(index_count, instance_count, first_index, vertex_offset,
                                                       first_instance);
        }

        static void on_resize(const WindowResizeEvent& e)
        {
            state->device->wait_idle();

            state->swapchain->resize({e.width, e.height});
        }

        void on_event(const Event& e) { mag::dispatch_event<WindowResizeEvent>(e, on_resize); }
    };  // namespace gfx
};  // namespace mag
