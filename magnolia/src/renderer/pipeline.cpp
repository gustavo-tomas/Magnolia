#include "renderer/pipeline.hpp"

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    void Pipeline::initialize(const vk::PipelineRenderingCreateInfo pipeline_rendering_create_info,
                              const std::vector<vk::DescriptorSetLayout>& descriptor_set_layouts,
                              const std::vector<Shader>& shaders, const VertexInputDescription& vertex_description,
                              const vec2& size, const vk::PipelineColorBlendAttachmentState& color_blend_attachment)
    {
        auto& context = get_context();

        vk::PipelineLayoutCreateInfo pipeline_layout_create_info({}, descriptor_set_layouts);
        this->pipeline_layout = context.get_device().createPipelineLayout(pipeline_layout_create_info);

        std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
        for (const auto& shader : shaders)
        {
            const vk::PipelineShaderStageCreateInfo create_info(
                {}, static_cast<vk::ShaderStageFlagBits>(shader.get_reflection().shader_stage), shader.get_handle(),
                "main");
            shader_stages.push_back(create_info);
        }

        const vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info({}, vertex_description.bindings,
                                                                                    vertex_description.attributes);

        const vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info(
            {}, vk::PrimitiveTopology::eTriangleList, false);

        const vk::Viewport viewport(0.0f, 0.0f, size.x, size.y, 0.0f, 1.0f);
        const vk::Rect2D scissor({}, {static_cast<u32>(size.x), static_cast<u32>(size.y)});

        const vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info(
            {}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise, false,
            0.0f, 0.0f, 0.0f, 1.0f);

        const vk::PipelineMultisampleStateCreateInfo multisampling_state_create_info({}, context.get_msaa_samples(),
                                                                                     false, 1.0f);

        const vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info(
            {}, true, true, vk::CompareOp::eLessOrEqual, {}, {}, {}, {}, 0.0f, 1.0f);

        const vk::PipelineViewportStateCreateInfo viewport_state({}, viewport, scissor);

        const vk::PipelineColorBlendStateCreateInfo color_blending({}, {}, vk::LogicOp::eCopy, color_blend_attachment);

        const std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        const vk::PipelineDynamicStateCreateInfo dynamic_state({}, dynamic_states);

        const vk::GraphicsPipelineCreateInfo pipeline_create_info(
            vk::PipelineCreateFlagBits::eDescriptorBufferEXT, shader_stages, &vertex_input_state_create_info,
            &input_assembly_create_info, {}, &viewport_state, &rasterization_state_create_info,
            &multisampling_state_create_info, &depth_stencil_create_info, &color_blending, &dynamic_state,
            pipeline_layout, {}, {}, {}, {}, &pipeline_rendering_create_info);

        const auto result_value = context.get_device().createGraphicsPipeline({}, pipeline_create_info);
        VK_CHECK(result_value.result);

        this->pipeline = result_value.value;
    }

    void Pipeline::shutdown()
    {
        auto& context = get_context();
        context.get_device().destroyPipeline(pipeline);
        context.get_device().destroyPipelineLayout(pipeline_layout);
    }
};  // namespace mag
