#pragma once

#include <vulkan/vulkan.hpp>

#include "renderer/shader.hpp"

namespace mag
{
    using namespace mag::math;

    class Pipeline
    {
        public:
            Pipeline(
                const vk::PipelineRenderingCreateInfo pipeline_rendering_create_info,
                const std::vector<vk::DescriptorSetLayout>& descriptor_set_layouts, const Shader& shader,
                const vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info = default_input_assembly(),
                const vk::PipelineRasterizationStateCreateInfo rasterization_state = default_rasterization_state(),
                const vk::PipelineColorBlendAttachmentState& color_blend_attachment = default_color_blend_attachment());

            ~Pipeline();

            void bind();

            const vk::PipelineLayout& get_layout() const { return pipeline_layout; };

            static vk::PipelineColorBlendAttachmentState const default_color_blend_attachment()
            {
                return vk::PipelineColorBlendAttachmentState(
                    {}, {}, {}, {}, {}, {}, {},
                    vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                        vk::ColorComponentFlagBits::eA);
            }

            static vk::PipelineInputAssemblyStateCreateInfo const default_input_assembly()
            {
                return vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, false);
            }

            static vk::PipelineRasterizationStateCreateInfo const default_rasterization_state()
            {
                return vk::PipelineRasterizationStateCreateInfo(
                    {}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
                    vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f);
            }

        private:
            vk::Pipeline pipeline;
            vk::PipelineLayout pipeline_layout;
    };
};  // namespace mag
