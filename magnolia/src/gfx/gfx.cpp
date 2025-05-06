#include "gfx/gfx.hpp"

#include "gfx/backend/backend.hpp"

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
        };

        struct GfxState
        {
                unique<IDevice> device;
                unique<ISwapchain> swapchain;
                unique<IQueue> graphics_queue;
                unique<IQueue> present_queue;
                unique<IGraphicsPipeline> graphics_pipeline;
                std::vector<FrameData> frames;
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

            // Graphics Pipeline
            // -------------------------------------------------------------------------------------------------
            IGraphicsPipelineDesc graphics_pipeline_desc = {};
            graphics_pipeline_desc.primitive_topology = PrimitiveTopology::TriangleList;
            graphics_pipeline_desc.format = state->swapchain->get_format();
            graphics_pipeline_desc.extent = state->swapchain->get_extent();
            state->graphics_pipeline = state->device->create_graphics_pipeline(graphics_pipeline_desc);

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
            }

            return state->device != nullptr;
        }

        void shutdown()
        {
            state->device->wait_idle();

            delete state;
        }

        void on_update(const f32 dt)
        {
            (void)dt;

            u32& current_frame = state->current_frame;
            state->frames[current_frame].in_flight_fence->wait();

            state->swapchain->acquire_next_image(state->frames[current_frame].available_semaphore.get());
            const u32 image_index = state->swapchain->get_current_image_index();

            // Render Passes
            // -------------------------------------------------------------------------------------------------
            unique<IRenderPass> render_pass;
            unique<IRenderingAttachment> color_attachment;

            IRenderingAttachmentDesc color_attachment_desc = {};
            color_attachment_desc.type = RenderingAttachmentType::Color;
            color_attachment_desc.clear_color = {0.4f, 0.6f, 0.8f, 1.0f};
            color_attachment_desc.texture = state->swapchain->get_texture(image_index);
            color_attachment = state->device->create_render_attachment(color_attachment_desc);

            IRenderPassDesc render_pass_desc = {};
            render_pass_desc.extent = state->swapchain->get_extent();
            render_pass_desc.color_attachments.push_back(color_attachment.get());
            render_pass = state->device->create_render_pass(render_pass_desc);

            state->frames[current_frame].command_buffer->begin_recording();

            state->frames[current_frame].command_buffer->pipeline_barrier(
                state->swapchain->get_texture(image_index), TextureLayout::ColorAttachment, AccessMask::None,
                AccessMask::ColorAttachmentWrite, PipelineStage::TopOfPipe, PipelineStage::ColorAttachmentOutput);

            state->frames[current_frame].command_buffer->set_viewport(state->swapchain->get_extent());
            state->frames[current_frame].command_buffer->set_scissor(state->swapchain->get_extent());

            state->frames[current_frame].command_buffer->begin_rendering(render_pass.get());

            state->frames[current_frame].command_buffer->bind_pipeline(state->graphics_pipeline.get());

            state->frames[current_frame].command_buffer->draw(3);

            state->frames[current_frame].command_buffer->end_rendering();

            state->frames[current_frame].command_buffer->pipeline_barrier(
                state->swapchain->get_texture(image_index), TextureLayout::Present, AccessMask::ColorAttachmentWrite,
                AccessMask::None, PipelineStage::ColorAttachmentOutput, PipelineStage::BottomOfPipe);

            state->frames[current_frame].command_buffer->end_recording();

            state->graphics_queue->submit(state->frames[current_frame].available_semaphore.get(),
                                          state->frames[current_frame].finished_semaphore.get(),
                                          state->frames[current_frame].in_flight_fence.get(),
                                          state->frames[current_frame].command_buffer.get());

            state->present_queue->present(state->swapchain.get(),
                                          state->frames[current_frame].finished_semaphore.get());

            current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
    };  // namespace gfx
};      // namespace mag
