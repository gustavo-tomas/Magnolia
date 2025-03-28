#include "renderer/pipeline.hpp"

#include <vulkan/vulkan.hpp>

#include "private/renderer_type_conversions.hpp"
#include "renderer/context.hpp"
#include "renderer/frame.hpp"
#include "renderer/shader.hpp"
#include "spirv_reflect.h"

namespace mag
{
    struct Pipeline::IMPL
    {
            IMPL() = default;
            ~IMPL() = default;

            vk::Pipeline pipeline;
            vk::PipelineLayout pipeline_layout;
    };

    Pipeline::Pipeline(const Shader& shader) : impl(new IMPL())
    {
        auto& context = get_context();

        const ShaderConfiguration& shader_configuration = shader.get_shader_configuration();

        // Input assembly

        vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info =
            vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, false);

        {
            input_assembly_create_info.setTopology(str_to_vk_topology(shader_configuration.topology));
        }

        // Rasterization

        vk::PipelineRasterizationStateCreateInfo rasterization_create_info(
            {}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
            false, 0.0f, 0.0f, 0.0f, 1.0f);

        {
            rasterization_create_info.setPolygonMode(str_to_vk_polygon_mode(shader_configuration.polygon_mode));
            rasterization_create_info.setCullMode(str_to_vk_cull_mode(shader_configuration.cull_mode));
        }

        // Color blend

        vk::PipelineColorBlendAttachmentState color_blend_attachment(
            {}, {}, {}, {}, {}, {}, {},
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA);

        {
            const b8 enabled = shader_configuration.color_blend_enabled;
            color_blend_attachment.setBlendEnable(enabled);

            if (enabled)
            {
                color_blend_attachment

                    .setColorBlendOp(str_to_vk_blend_op(shader_configuration.color_blend_op))
                    .setAlphaBlendOp(str_to_vk_blend_op(shader_configuration.alpha_blend_op))
                    .setSrcColorBlendFactor(str_to_vk_blend_factor(shader_configuration.src_color_blend_factor))
                    .setDstColorBlendFactor(str_to_vk_blend_factor(shader_configuration.dst_color_blend_factor))
                    .setSrcAlphaBlendFactor(str_to_vk_blend_factor(shader_configuration.src_alpha_blend_factor))
                    .setDstAlphaBlendFactor(str_to_vk_blend_factor(shader_configuration.dst_alpha_blend_factor));
            }
        }

        vk::PipelineLayoutCreateInfo pipeline_layout_create_info({}, shader.get_descriptor_set_layouts(),
                                                                 shader.get_push_constant_ranges());

        impl->pipeline_layout = context.get_device().createPipelineLayout(pipeline_layout_create_info);

        const auto& shader_modules = shader_configuration.shader_modules;

        std::vector<vk::PipelineShaderStageCreateInfo> shader_stages;
        for (const auto& shader_module : shader_modules)
        {
            const vk::PipelineShaderStageCreateInfo create_info(
                {}, static_cast<vk::ShaderStageFlagBits>(shader_module.spv_module->shader_stage), *shader_module.module,
                "main");

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

        const auto color_format = context.get_supported_color_format(ImageFormat::Float);

        std::vector<vk::Format> color_attachment_formats;
        if (shader_configuration.color_write_enabled)
        {
            color_attachment_formats.push_back(color_format);
        }

        vk::Format depth_format = {};
        if (shader_configuration.depth_write_enabled)
        {
            depth_format = context.get_supported_depth_format();
        }

        // Create pipeline
        const vk::PipelineRenderingCreateInfo pipeline_rendering_create_info({}, color_attachment_formats,
                                                                             depth_format);

        const vk::GraphicsPipelineCreateInfo pipeline_create_info(
            {}, shader_stages, &vertex_input_state_create_info, &input_assembly_create_info, {}, &viewport_state,
            &rasterization_create_info, &multisampling_state_create_info, &depth_stencil_create_info, &color_blending,
            &dynamic_state, impl->pipeline_layout, {}, {}, {}, {}, &pipeline_rendering_create_info);

        const auto result_value = context.get_device().createGraphicsPipeline({}, pipeline_create_info);
        VK_CHECK(result_value.result);

        impl->pipeline = result_value.value;
    }

    Pipeline::~Pipeline()
    {
        auto& context = get_context();
        context.get_device().destroyPipeline(impl->pipeline);
        context.get_device().destroyPipelineLayout(impl->pipeline_layout);
    }

    void Pipeline::bind()
    {
        const CommandBuffer& command_buffer = get_context().get_curr_frame().command_buffer;
        command_buffer.get_handle().bindPipeline(vk::PipelineBindPoint::eGraphics, impl->pipeline);
    }

    const void* Pipeline::get_layout() const { return &impl->pipeline_layout; }
};  // namespace mag
