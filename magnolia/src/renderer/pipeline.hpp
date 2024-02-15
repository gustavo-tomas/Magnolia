#pragma once

#include <vulkan/vulkan.hpp>

#include "renderer/model.hpp"
#include "renderer/shader.hpp"

namespace mag
{
    using namespace mag::math;

    class Pipeline
    {
        public:
            void initialize(
                const vk::RenderPass& render_pass, const std::vector<vk::DescriptorSetLayout>& descriptor_set_layouts,
                const std::vector<Shader>& shaders, const VertexInputDescription& vertex_description, const vec2& size,
                const vk::PipelineColorBlendAttachmentState& color_blend_attachment = default_color_blend_attachment());

            void shutdown();

            const vk::Pipeline& get_handle() const { return pipeline; };
            const vk::PipelineLayout& get_layout() const { return pipeline_layout; };

            static vk::PipelineColorBlendAttachmentState const default_color_blend_attachment()
            {
                return vk::PipelineColorBlendAttachmentState(
                    {}, {}, {}, {}, {}, {}, {},
                    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                        vk::ColorComponentFlagBits::eA);
            }

        private:
            vk::Pipeline pipeline;
            vk::PipelineLayout pipeline_layout;
    };
};  // namespace mag
