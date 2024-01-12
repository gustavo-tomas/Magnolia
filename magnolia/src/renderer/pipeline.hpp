#pragma once

#include "core/math.hpp"
#include "renderer/shader.hpp"

namespace mag
{
    class Pipeline
    {
        public:
            void initialize(const vk::RenderPass& render_pass,
                            const std::vector<vk::DescriptorSetLayout>& descriptor_set_layouts,
                            const std::vector<Shader>& shaders, const vec2& size);
            void shutdown();

            const vk::Pipeline& get_handle() const { return pipeline; };
            const vk::PipelineLayout& get_layout() const { return pipeline_layout; };

        private:
            vk::Pipeline pipeline;
            vk::PipelineLayout pipeline_layout;
    };
};  // namespace mag
