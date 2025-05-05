#include "gfx/gfx.hpp"

#include "gfx/backend/backend.hpp"

namespace mag
{
    namespace gfx
    {
        // @TODO: temporary
#define MAX_FRAMES_IN_FLIGHT 3

        struct GfxState
        {
                unique<IDevice> device;
                unique<ISwapchain> swapchain;
                unique<IQueue> graphics_queue;
                unique<IQueue> present_queue;
                unique<IGraphicsPipeline> graphics_pipeline;
                std::vector<unique<ICommandBuffer>> command_buffers;
                std::vector<unique<IRenderPass>> render_passes;
                std::vector<unique<IRenderingAttachment>> color_attachments;
                std::vector<unique<ISemaphore>> available_semaphores;
                std::vector<unique<ISemaphore>> finished_semaphore;
                std::vector<unique<IFence>> in_flight_fences;
                std::vector<IFence*> image_in_flight;
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

            // Command Buffers
            // -------------------------------------------------------------------------------------------------
            for (u32 i = 0; i < state->swapchain->get_image_count(); i++)
            {
                ICommandBufferDesc desc = {};
                desc.command_buffer_level = CommandBufferLevel::Primary;
                state->command_buffers.push_back(state->device->create_command_buffer(desc));
            }

            // Render Passes
            // -------------------------------------------------------------------------------------------------
            state->color_attachments.resize(state->command_buffers.size());
            state->render_passes.resize(state->command_buffers.size());

            for (u64 i = 0; i < state->command_buffers.size(); i++)
            {
                state->command_buffers[i]->begin_recording();

                IRenderingAttachmentDesc color_attachment_desc = {};
                color_attachment_desc.type = RenderingAttachmentType::Color;
                color_attachment_desc.clear_color = {0.4f, 0.6f, 0.8f, 1.0f};
                color_attachment_desc.texture = state->swapchain->get_texture(i);
                state->color_attachments[i] = state->device->create_render_attachment(color_attachment_desc);

                IRenderPassDesc render_pass_desc = {};
                render_pass_desc.extent = state->swapchain->get_extent();
                render_pass_desc.color_attachments.push_back(state->color_attachments[i].get());
                state->render_passes[i] = state->device->create_render_pass(render_pass_desc);

                state->command_buffers[i]->pipeline_barrier(
                    state->swapchain->get_texture(i), TextureLayout::ColorAttachment, AccessMask::None,
                    AccessMask::ColorAttachmentWrite, PipelineStage::TopOfPipe, PipelineStage::ColorAttachmentOutput);

                state->command_buffers[i]->set_viewport(state->swapchain->get_extent());
                state->command_buffers[i]->set_scissor(state->swapchain->get_extent());

                state->command_buffers[i]->begin_rendering(state->render_passes[i].get());

                state->command_buffers[i]->bind_pipeline(state->graphics_pipeline.get());

                state->command_buffers[i]->draw(3);

                state->command_buffers[i]->end_rendering();

                state->command_buffers[i]->pipeline_barrier(
                    state->swapchain->get_texture(i), TextureLayout::Present, AccessMask::ColorAttachmentWrite,
                    AccessMask::None, PipelineStage::ColorAttachmentOutput, PipelineStage::BottomOfPipe);

                state->command_buffers[i]->end_recording();
            }

            // Sync Objects
            // -------------------------------------------------------------------------------------------------
            state->available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
            state->finished_semaphore.resize(MAX_FRAMES_IN_FLIGHT);
            state->in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
            state->image_in_flight.resize(state->swapchain->get_image_count());

            IFenceDesc fence_desc = {};
            fence_desc.signaled = true;

            for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                state->available_semaphores[i] = state->device->create_semaphore();
                state->finished_semaphore[i] = state->device->create_semaphore();
                state->in_flight_fences[i] = state->device->create_fence(fence_desc);
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
            state->in_flight_fences[current_frame]->wait();

            state->swapchain->acquire_next_image(state->available_semaphores[current_frame].get());
            const u32 image_index = state->swapchain->get_current_image_index();

            if (state->image_in_flight[image_index] != nullptr)
            {
                state->image_in_flight[image_index]->wait();
            }
            state->image_in_flight[image_index] = state->in_flight_fences[current_frame].get();

            state->graphics_queue->submit(
                state->available_semaphores[current_frame].get(), state->finished_semaphore[current_frame].get(),
                state->in_flight_fences[current_frame].get(), state->command_buffers[image_index].get());

            state->present_queue->present(state->swapchain.get(), state->finished_semaphore[current_frame].get());

            current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
    };  // namespace gfx
};      // namespace mag
