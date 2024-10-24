#pragma once

#include "core/types.hpp"
#include "renderer/command.hpp"

namespace mag
{
    class Context;
    class RendererImage;

    struct Frame
    {
            vk::Fence render_fence;
            vk::Semaphore render_semaphore;
            vk::Semaphore present_semaphore;
            CommandBuffer command_buffer;
    };

    class FrameProvider
    {
        public:
            void initialize(const u32 frame_count);
            void shutdown();

            void begin_frame();
            b8 end_frame(const RendererImage& draw_image, const vk::Extent3D& extent);

            Frame& get_current_frame() { return frames[frame_number]; }
            const u32& get_swapchain_image_index() const { return this->swapchain_image_index; };
            const u32& get_current_frame_number() const { return frame_number; };

        private:
            std::vector<Frame> frames;
            u32 frame_number = {};
            u32 swapchain_image_index = {};
    };
};  // namespace mag
