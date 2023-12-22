#pragma once

#include <vulkan/vulkan.hpp>

#include "core/math.hpp"
#include "renderer/pipeline.hpp"

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
            virtual void render(const CommandBuffer& /* command_buffer */){};

            virtual Pass& get_pass() = 0;
    };

    class StandardRenderPass : public RenderPass
    {
        public:
            virtual void initialize(const uvec2& size) override;
            virtual void shutdown() override;
            virtual void render(const CommandBuffer& command_buffer) override;

            virtual Pass& get_pass() override;

            void on_resize(const uvec2& size);

        private:
            Pass pass = {};
            Pipeline triangle_pipeline;
            Shader triangle_vs, triangle_fs;
            std::vector<vk::Framebuffer> frame_buffers;
    };
};  // namespace mag
