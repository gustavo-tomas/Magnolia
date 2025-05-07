#include "gfx/gfx.hpp"

#include "core/assert.hpp"
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

        struct FrameData
        {
                unique<ICommandPool> command_pool;
                unique<ICommandBuffer> command_buffer;
                unique<ISemaphore> available_semaphore;
                unique<ISemaphore> finished_semaphore;
                unique<IFence> in_flight_fence;
                unique<ITexture> render_target;
        };

        struct GfxState
        {
                unique<IDevice> device;
                unique<ISwapchain> swapchain;
                unique<IQueue> graphics_queue;
                unique<IQueue> present_queue;
                std::vector<FrameData> frames;
                std::map<ShaderHandle, unique<IGraphicsPipeline>> shaders;
                u32 current_frame = 0;
        };

        static GfxState* state = nullptr;

        static ShaderHandle create_handle()
        {
            // @TODO: this is a pretty simple way to create handles, but it works
            static ShaderHandle handle_counter = 0;

            return handle_counter++;
        }

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

        ShaderHandle create_shader(const ShaderResource& shader)
        {
            // Graphics Pipeline
            // -------------------------------------------------------------------------------------------------
            IGraphicsPipelineDesc graphics_pipeline_desc = {};
            graphics_pipeline_desc.primitive_topology = PrimitiveTopology::TriangleList;
            graphics_pipeline_desc.format = state->swapchain->get_format();
            graphics_pipeline_desc.extent = state->swapchain->get_extent();

            for (const auto& [shader_stage, code] : shader.stages)
            {
                IShaderModuleDesc shader_module_desc = {};
                shader_module_desc.code = code;

                if (shader_stage == ShaderResourceStage::Vertex)
                {
                    shader_module_desc.stage = ShaderStage::Vertex;
                }

                else if (shader_stage == ShaderResourceStage::Fragment)
                {
                    shader_module_desc.stage = ShaderStage::Fragment;
                }

                else
                {
                    MAG_ASSERT(false, "Unhandled shader stage");
                }

                graphics_pipeline_desc.shader_modules.push_back(shader_module_desc);
            }

            const ShaderHandle handle = create_handle();
            state->shaders[handle] = state->device->create_graphics_pipeline(graphics_pipeline_desc);

            return handle;
        }

        void use_shader(const ShaderHandle& handle)
        {
            u32& current_frame_idx = state->current_frame;
            FrameData& current_frame = state->frames[current_frame_idx];

            current_frame.command_buffer->bind_pipeline(state->shaders[handle].get());
        }

        void draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex, const u32 first_instance)
        {
            u32& current_frame_idx = state->current_frame;
            FrameData& current_frame = state->frames[current_frame_idx];

            current_frame.command_buffer->draw(vertex_count, instance_count, first_vertex, first_instance);
        }
    };  // namespace gfx
};      // namespace mag
