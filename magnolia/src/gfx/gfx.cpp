#include "gfx/gfx.hpp"

#include <map>

#include "core/types.hpp"
#include "gfx/backend/backend.hpp"
#include "resources/resource_loader.hpp"
#include "resources/shader.hpp"

namespace mag
{
    namespace gfx
    {
        // @TODO: temporary
#define MAX_FRAMES_IN_FLIGHT 3

        struct DescriptorData
        {
                unique<IDescriptorSet> descriptor_set;
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
                unique<ITexture> render_target;
                unique<IDescriptorPool> descriptor_pool;
                std::map<ShaderHandle, DescriptorData> descriptor_set_map;
        };

        struct GfxState
        {
                unique<IDevice> device;
                unique<ISwapchain> swapchain;
                unique<IQueue> graphics_queue;
                unique<IQueue> present_queue;
                std::vector<FrameData> frames;
                std::map<ShaderHandle, ShaderData> shaders;
                std::map<BufferHandle, unique<IBuffer>> buffers;
                std::map<TextureHandle, TextureData> textures;
                u32 current_frame = 0;
        };

        static GfxState* state = nullptr;

        b8 initialize()
        {
            state = new GfxState();
            state->device = create_device();

            // Swapchain
            // -------------------------------------------------------------------------------------------------
            ISwapchainDesc swapchain_desc = {};
            swapchain_desc.desired_present_mode = PresentMode::Mailbox;
            state->swapchain = state->device->create_swapchain(swapchain_desc);

            // Queues
            // -------------------------------------------------------------------------------------------------
            state->graphics_queue = state->device->create_queue({.queue_type = QueueType::Graphics});
            state->present_queue = state->device->create_queue({.queue_type = QueueType::Present});

            // Command Pool, Command Buffers and Sync Objects
            // -------------------------------------------------------------------------------------------------
            state->frames.resize(MAX_FRAMES_IN_FLIGHT);

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

                ITextureDesc texture_desc = {};
                texture_desc.extent = math::uvec3(state->swapchain->get_extent(), 1.0f);
                texture_desc.usage = TextureUsage::ColorAttachment | TextureUsage::TransferSrc;
                state->frames[i].render_target = state->device->create_texture(texture_desc);

                IDescriptorPoolDesc descriptor_pool_desc = {};
                descriptor_pool_desc.max_sets = 1024;

                IDescriptorPoolSizeDesc size_desc_uniform = {};
                size_desc_uniform.type = DescriptorType::Uniform;
                size_desc_uniform.size = 64;

                IDescriptorPoolSizeDesc size_desc_combined_sampler = {};
                size_desc_combined_sampler.type = DescriptorType::CombinedImageSampler;
                size_desc_combined_sampler.size = 64;

                descriptor_pool_desc.size_descs.push_back(size_desc_uniform);
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

        void begin_frame()
        {
            FrameData& current_frame = state->frames[state->current_frame];
            const unique<ITexture>& render_target = current_frame.render_target;

            current_frame.in_flight_fence->wait();
            state->swapchain->acquire_next_image(current_frame.available_semaphore.get());

            // Render Passes
            // -------------------------------------------------------------------------------------------------
            unique<IRenderPass> render_pass;
            unique<IRenderingAttachment> color_attachment;

            IRenderingAttachmentDesc color_attachment_desc = {};
            color_attachment_desc.type = RenderingAttachmentType::Color;
            color_attachment_desc.clear_color = {0.4f, 0.6f, 0.8f, 1.0f};
            color_attachment_desc.texture = render_target.get();
            color_attachment = state->device->create_render_attachment(color_attachment_desc);

            IRenderPassDesc render_pass_desc = {};
            render_pass_desc.extent = state->swapchain->get_extent();
            render_pass_desc.color_attachments.push_back(color_attachment.get());
            render_pass = state->device->create_render_pass(render_pass_desc);

            current_frame.command_buffer->begin_recording();

            // Prepare render target for rendering
            current_frame.command_buffer->pipeline_barrier(
                render_target.get(), TextureLayout::ColorAttachment, AccessMask::None, AccessMask::ColorAttachmentWrite,
                PipelineStage::TopOfPipe, PipelineStage::ColorAttachmentOutput);

            current_frame.command_buffer->set_viewport(state->swapchain->get_extent());
            current_frame.command_buffer->set_scissor(state->swapchain->get_extent());

            current_frame.command_buffer->begin_rendering(render_pass.get());
        }

        void end_frame()
        {
            u32& current_frame_idx = state->current_frame;
            FrameData& current_frame = state->frames[current_frame_idx];
            const unique<ITexture>& render_target = current_frame.render_target;

            const u32 image_index = state->swapchain->get_current_image_index();

            current_frame.command_buffer->end_rendering();

            // Transition render target to transfer
            current_frame.command_buffer->pipeline_barrier(
                render_target.get(), TextureLayout::TransferSrc, AccessMask::ColorAttachmentWrite,
                AccessMask::TransferRead, PipelineStage::ColorAttachmentOutput, PipelineStage::Transfer);

            // Transition swapchain image to transfer
            current_frame.command_buffer->pipeline_barrier(
                state->swapchain->get_texture(image_index), TextureLayout::TransferDst, AccessMask::None,
                AccessMask::TransferWrite, PipelineStage::TopOfPipe, PipelineStage::Transfer);

            // Copy from the render target to the swapchain image
            current_frame.command_buffer->copy_texture(render_target.get(), state->swapchain->get_texture(image_index));

            // Transition swapchain image to present
            current_frame.command_buffer->pipeline_barrier(
                state->swapchain->get_texture(image_index), TextureLayout::Present, AccessMask::TransferWrite,
                AccessMask::MemoryRead, PipelineStage::Transfer, PipelineStage::BottomOfPipe);

            current_frame.command_buffer->end_recording();

            state->graphics_queue->submit(current_frame.available_semaphore.get(),
                                          current_frame.finished_semaphore.get(), current_frame.in_flight_fence.get(),
                                          current_frame.command_buffer.get());

            state->present_queue->present(state->swapchain.get(), current_frame.finished_semaphore.get());

            current_frame_idx = (current_frame_idx + 1) % MAX_FRAMES_IN_FLIGHT;
        }

        static gfx::ShaderStage convert_resource_shader_stage(const ShaderResourceStage shader_stage)
        {
            switch (shader_stage)
            {
                case ShaderResourceStage::Vertex:
                    return gfx::ShaderStage::Vertex;
                    break;

                case ShaderResourceStage::Fragment:
                    return gfx::ShaderStage::Fragment;
                    break;
            }
        }

        static u32 create_handle()
        {
            // @TODO: this is a pretty simple way to create handles, but it works
            static u32 handle_counter = 0;

            return handle_counter++;
        }

        BufferHandle create_buffer(const u64 size, const void* data)
        {
            const BufferHandle handle = create_handle();

            IBufferDesc buffer_desc = {};
            buffer_desc.buffer_usage = BufferUsage::Uniform;
            buffer_desc.memory_usage = MemoryUsage::Auto;
            buffer_desc.size_bytes = size;

            state->buffers[handle] = state->device->create_buffer(buffer_desc);
            state->buffers[handle]->set_data(data, size);

            return handle;
        }

        void set_buffer_data(const BufferHandle buffer_handle, const u64 size, const void* data)
        {
            state->buffers[buffer_handle]->set_data(data, size);
        }

        TextureHandle create_texture(const u32 width, const u32 height, const u64 size, const void* pixels)
        {
            const TextureHandle handle = create_handle();

            ITextureDesc texture_desc = {};
            texture_desc.extent = math::uvec3(width, height, 1);
            texture_desc.usage = TextureUsage::ColorAttachment | TextureUsage::Sampled | TextureUsage::TransferDst;

            ISamplerDesc sampler_desc = {};
            sampler_desc.min_lod = 0.0f;
            sampler_desc.max_lod = texture_desc.mip_levels;

            state->textures[handle].sampler = state->device->create_sampler(sampler_desc);
            state->textures[handle].texture = state->device->create_texture(texture_desc);

            if (size && pixels)
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

            IDescriptorSetLayoutBindingDesc binding_desc0 = {};
            binding_desc0.binding = 0;
            binding_desc0.descriptor_count = 1;
            binding_desc0.descriptor_type = DescriptorType::Uniform;

            // @TODO: this is hardcoded to make my life easier
            binding_desc0.stages = ShaderStage::Vertex | ShaderStage::Fragment;

            IDescriptorSetLayoutBindingDesc binding_desc1 = {};
            binding_desc1.binding = 1;
            binding_desc1.descriptor_count = 1;
            binding_desc1.descriptor_type = DescriptorType::CombinedImageSampler;

            // @TODO: this is hardcoded to make my life easier
            binding_desc1.stages = ShaderStage::Vertex | ShaderStage::Fragment;

            descriptor_layout_desc.binding_descs.push_back(binding_desc0);
            descriptor_layout_desc.binding_descs.push_back(binding_desc1);

            state->shaders[handle].descriptor_layout =
                state->device->create_descriptor_set_layout(descriptor_layout_desc);

            const unique<IDescriptorSetLayout>& descriptor_layout = state->shaders[handle].descriptor_layout;

            // Descriptor set

            for (u32 i = 0; i < state->frames.size(); i++)
            {
                IDescriptorSetDesc descriptor_desc = {};
                descriptor_desc.descriptor_layout = descriptor_layout.get();
                descriptor_desc.descriptor_pool = state->frames[i].descriptor_pool.get();
                descriptor_desc.max_descriptor_count = 1;

                state->frames[i].descriptor_set_map[handle].descriptor_set =
                    state->device->create_descriptor_set(descriptor_desc);
            }

            // Graphics Pipeline
            // -------------------------------------------------------------------------------------------------
            IGraphicsPipelineDesc graphics_pipeline_desc = {};
            graphics_pipeline_desc.primitive_topology = PrimitiveTopology::TriangleList;
            graphics_pipeline_desc.format = state->swapchain->get_format();
            graphics_pipeline_desc.extent = state->swapchain->get_extent();
            graphics_pipeline_desc.descriptor_layouts.push_back(descriptor_layout.get());

            for (const auto& [shader_stage, code] : shader.stages)
            {
                IShaderModuleDesc shader_module_desc = {};
                shader_module_desc.code = code;
                shader_module_desc.stage = convert_resource_shader_stage(shader_stage);

                graphics_pipeline_desc.shader_modules.push_back(shader_module_desc);
            }

            state->shaders[handle].pipeline = state->device->create_graphics_pipeline(graphics_pipeline_desc);

            return handle;
        }

        void set_shader_buffer_uniform(const ShaderHandle shader_handle, const BufferHandle buffer_handle,
                                       const u32 binding, const u32 array_element)
        {
            FrameData& current_frame = state->frames[state->current_frame];

            const ShaderData& shader = state->shaders[shader_handle];
            const unique<IBuffer>& buffer = state->buffers[buffer_handle];
            DescriptorData& descriptor_data = current_frame.descriptor_set_map[shader_handle];

            // If we change the buffer, we need to update the descriptor sets (for each frame)

            if (descriptor_data.last_bound_buffer != buffer_handle)
            {
                descriptor_data.descriptor_set->update(buffer.get(), binding, array_element, DescriptorType::Uniform);
                descriptor_data.last_bound_buffer = buffer_handle;
            }

            current_frame.command_buffer->bind_descriptor(shader.pipeline.get(), descriptor_data.descriptor_set.get());
        }

        void set_shader_texture_uniform(const ShaderHandle shader_handle, const TextureHandle texture_handle,
                                        const u32 binding, const u32 array_element)
        {
            FrameData& current_frame = state->frames[state->current_frame];

            const ShaderData& shader = state->shaders[shader_handle];
            const unique<ITexture>& texture = state->textures[texture_handle].texture;
            const unique<ISampler>& sampler = state->textures[texture_handle].sampler;
            DescriptorData& descriptor_data = current_frame.descriptor_set_map[shader_handle];

            // If we change the texture, we need to update the descriptor sets (for each frame)

            if (descriptor_data.last_bound_texture != texture_handle)
            {
                descriptor_data.descriptor_set->update(texture.get(), sampler.get(), binding, array_element,
                                                       DescriptorType::CombinedImageSampler);
                descriptor_data.last_bound_texture = texture_handle;
            }

            current_frame.command_buffer->bind_descriptor(shader.pipeline.get(), descriptor_data.descriptor_set.get());
        }

        void use_shader(const ShaderHandle& handle)
        {
            FrameData& current_frame = state->frames[state->current_frame];

            current_frame.command_buffer->bind_pipeline(state->shaders[handle].pipeline.get());
        }

        void draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex, const u32 first_instance)
        {
            FrameData& current_frame = state->frames[state->current_frame];

            current_frame.command_buffer->draw(vertex_count, instance_count, first_vertex, first_instance);
        }
    };  // namespace gfx
};      // namespace mag
