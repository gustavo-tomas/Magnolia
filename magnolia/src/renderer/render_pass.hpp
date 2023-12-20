#pragma once

#include <vulkan/vulkan.hpp>

#include "core/math.hpp"

namespace mag
{
    class CommandBuffer;

    struct Pass
    {
            vk::RenderPass render_pass;
            vk::Framebuffer frame_buffer;
            vk::PipelineBindPoint pipeline_bind_point;
            vk::Rect2D render_area = {};
            std::vector<vk::ClearValue> clear_values;
    };

    class RenderPass
    {
        public:
            virtual ~RenderPass() = default;

            virtual void initialize(const uvec2& /* size */){};
            virtual void shutdown(){};

            virtual Pass& get_pass() = 0;
    };

    class StandardRenderPass : public RenderPass
    {
        public:
            virtual void initialize(const uvec2& size) override;
            virtual void shutdown() override;

            virtual Pass& get_pass() override;

            void resize(const uvec2& size);

        private:
            Pass pass = {};
            std::vector<vk::Framebuffer> frame_buffers;
    };
};  // namespace mag
