#include "renderer/pipeline.hpp"

#include "core/assert.hpp"
#include "renderer/context.hpp"
#include "renderer/pipeline_type_conversions.hpp"

namespace mag
{
#define INPUT_ASSEMBLY "InputAssembly"
#define RASTERIZATION "Rasterization"
#define COLOR_BLEND "ColorBlend"

    Pipeline::Pipeline(const Shader& shader, const vk::PipelineRenderingCreateInfo pipeline_rendering_create_info,
                       const json pipeline_data)
    {
        auto& context = get_context();

        // Validity checks
        ASSERT(pipeline_data.contains(INPUT_ASSEMBLY), "Pipeline data missing input assembly configuration");
        ASSERT(pipeline_data.contains(RASTERIZATION), "Pipeline data missing rasterization configuration");
        ASSERT(pipeline_data.contains(COLOR_BLEND), "Pipeline data missing color blend configuration");

        const json input_assembly_data = pipeline_data[INPUT_ASSEMBLY];
        const json rasterization_data = pipeline_data[RASTERIZATION];
        const json color_blend_data = pipeline_data[COLOR_BLEND];

        // Input assembly

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info =
            vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, false);

        {
            input_assembly_create_info.setTopology(str_to_vk_topology(input_assembly_data["Topology"]));
        }

        // Rasterization

        vk::PipelineRasterizationStateCreateInfo rasterization_create_info(
            {}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
            false, 0.0f, 0.0f, 0.0f, 1.0f);

        {
            rasterization_create_info.setPolygonMode(str_to_vk_polygon_mode(rasterization_data["PolygonMode"]));
            rasterization_create_info.setCullMode(str_to_vk_cull_mode(rasterization_data["CullMode"]));
        }

        // Color blend

        vk::PipelineColorBlendAttachmentState color_blend_attachment(
            {}, {}, {}, {}, {}, {}, {},
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA);

        {
            const b8 enabled = color_blend_data["Enabled"].get<b8>();
            color_blend_attachment.setBlendEnable(enabled);

            if (enabled)
            {
                color_blend_attachment

                    .setColorBlendOp(str_to_vk_blend_op(color_blend_data["ColorBlendOp"]))
                    .setAlphaBlendOp(str_to_vk_blend_op(color_blend_data["AlphaBlendOp"]))
                    .setSrcColorBlendFactor(str_to_vk_blend_factor(color_blend_data["SrcColorBlendFactor"]))
                    .setDstColorBlendFactor(str_to_vk_blend_factor(color_blend_data["DstColorBlendFactor"]))
                    .setSrcAlphaBlendFactor(str_to_vk_blend_factor(color_blend_data["SrcAlphaBlendFactor"]))
                    .setDstAlphaBlendFactor(str_to_vk_blend_factor(color_blend_data["DstAlphaBlendFactor"]));
            }
        }

        vk::PipelineLayoutCreateInfo pipeline_layout_create_info({}, shader.get_descriptor_set_layouts());
        this->pipeline_layout = context.get_device().createPipelineLayout(pipeline_layout_create_info);

        const auto& shader_modules = shader.get_modules();

        std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
        for (const auto& shader_module : shader_modules)
        {
            const vk::PipelineShaderStageCreateInfo create_info(
                {}, static_cast<vk::ShaderStageFlagBits>(shader_module->get_reflection().shader_stage),
                shader_module->get_handle(), "main");

            shader_stages.push_back(create_info);
        }

        // Extract vertex input info from vertex shader
        const vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info({}, shader.get_vertex_bindings(),
                                                                                    shader.get_vertex_attributes());

        const vk::PipelineMultisampleStateCreateInfo multisampling_state_create_info({}, vk::SampleCountFlagBits::e1,
                                                                                     false, 1.0f);

        const vk::PipelineDepthStencilStateCreateInfo depth_stencil_create_info(
            {}, true, true, vk::CompareOp::eLessOrEqual, {}, {}, {}, {}, 0.0f, 1.0f);

        const vk::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
        const vk::Rect2D scissor({}, {1, 1});

        const vk::PipelineViewportStateCreateInfo viewport_state({}, viewport, scissor);

        const vk::PipelineColorBlendStateCreateInfo color_blending({}, {}, vk::LogicOp::eCopy, color_blend_attachment);

        const std::vector<vk::DynamicState> dynamic_states = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        const vk::PipelineDynamicStateCreateInfo dynamic_state({}, dynamic_states);

        const vk::GraphicsPipelineCreateInfo pipeline_create_info(
            {}, shader_stages, &vertex_input_state_create_info, &input_assembly_create_info, {}, &viewport_state,
            &rasterization_create_info, &multisampling_state_create_info, &depth_stencil_create_info, &color_blending,
            &dynamic_state, pipeline_layout, {}, {}, {}, {}, &pipeline_rendering_create_info);

        const auto result_value = context.get_device().createGraphicsPipeline({}, pipeline_create_info);
        VK_CHECK(result_value.result);

        this->pipeline = result_value.value;
    }

    Pipeline::~Pipeline()
    {
        auto& context = get_context();
        context.get_device().destroyPipeline(pipeline);
        context.get_device().destroyPipelineLayout(pipeline_layout);
    }

    void Pipeline::bind()
    {
        const CommandBuffer& command_buffer = get_context().get_curr_frame().command_buffer;
        command_buffer.get_handle().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    }
};  // namespace mag
