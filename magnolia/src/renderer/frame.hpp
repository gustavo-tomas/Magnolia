#pragma once

#include "core/types.hpp"
#include "renderer/command.hpp"

namespace mag
{
    class Context;

    struct Frame
    {
            vk::Fence render_fence;
            CommandBuffer command_buffer;
    };

    class FrameProvider
    {
        public:
            void initialize(const u32 frame_count);
            void shutdown();

            void begin_frame();
            void end_frame();

            Frame& get_current_frame() { return frames[frame_number]; }
            u32 get_swapchain_image_index() const { return this->swapchain_image_index; };

        private:
            std::vector<Frame> frames;
            u32 frame_number = {};
            u32 swapchain_image_index = {};
    };
};  // namespace mag
