#include "renderer/render_graph.hpp"

#include <vulkan/vulkan.hpp>

#include "core/assert.hpp"
#include "private/renderer_type_conversions.hpp"
#include "renderer/context.hpp"
#include "renderer/frame.hpp"
#include "tools/profiler.hpp"

// @TODO: reimplement missing features:
// - Multisampling
// - Render scale (just for the scene passes)

namespace mag
{
    RenderGraphPass::RenderGraphPass(const str& name) : name(name) {}
    RenderGraphPass::~RenderGraphPass() = default;

    void RenderGraphPass::on_render(RenderGraph& render_graph) { (void)render_graph; }

    void RenderGraphPass::add_input_attachment(const str& attachment_name, const AttachmentType attachment_type,
                                               const uvec2& size, const AttachmentState attachment_state)
    {
        AttachmentDescription attachment_description = {};
        attachment_description.name = attachment_name;
        attachment_description.type = attachment_type;
        attachment_description.size = size;
        attachment_description.stage = AttachmentStage::Input;
        attachment_description.state = attachment_state;

        add_attachment(attachment_description);
    }

    void RenderGraphPass::add_output_attachment(const str& attachment_name, const AttachmentType attachment_type,
                                                const uvec2& size, const AttachmentState attachment_state)
    {
        AttachmentDescription attachment_description = {};
        attachment_description.name = attachment_name;
        attachment_description.type = attachment_type;
        attachment_description.size = size;
        attachment_description.stage = AttachmentStage::Output;
        attachment_description.state = attachment_state;

        add_attachment(attachment_description);
    }

    void RenderGraphPass::add_attachment(const AttachmentDescription& attachment_description)
    {
        // @TODO: make it faster
        // Check if there are not repeated inputs or repeated outputs
        for (const auto& desc : attachment_descriptions)
        {
            if (desc.name == attachment_description.name && desc.stage == attachment_description.stage)
            {
                ASSERT(false,
                       "Pass '" + name + "' already contains the attachment '" + attachment_description.name + "'");
            }
        }

        attachment_descriptions.push_back(attachment_description);
    }

    const PerformanceResults& RenderGraphPass::get_performance_results() const { return performance_results; }
    const str& RenderGraphPass::get_name() const { return name; }

    // Render graph
    // -----------------------------------------------------------------------------------------------------------------
    RenderGraph::RenderGraph() = default;

    RenderGraph::~RenderGraph()
    {
        for (auto* pass : passes)
        {
            delete pass;
        }
    }

    void RenderGraph::add_pass(RenderGraphPass* render_pass)
    {
        const u32 frame_count = get_context().get_frame_count();

        for (const auto& attachment_desc : render_pass->attachment_descriptions)
        {
            if (!attachments.contains(attachment_desc.name))
            {
                attachments[attachment_desc.name] = {};
                attachments[attachment_desc.name].resize(frame_count);

                for (u32 i = 0; i < frame_count; i++)
                {
                    attachments[attachment_desc.name][i].description = attachment_desc;
                }
            }
        }

        passes.push_back(render_pass);
    }

    void RenderGraph::set_output_attachment(const str& attachment_name) { output_attachment_name = attachment_name; }

    void RenderGraph::build()
    {
        auto& context = get_context();

        const u32 frame_count = context.get_frame_count();

        // Build attachments
        for (auto& attachment_p : attachments)
        {
            auto& attachment = attachment_p.second;
            const auto& description = attachment[0].description;

            vk::ImageAspectFlags image_aspect = {};
            vk::ImageUsageFlags image_usage = {};
            vk::Format image_format = {};
            const uvec3 image_extent = uvec3(description.size, 1);

            switch (description.type)
            {
                // @TODO: hardcoded sampled, trasfersrc, dst
                case AttachmentType::Color:
                    image_aspect = vk::ImageAspectFlagBits::eColor;
                    image_usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled |
                                  vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
                    image_format = vk::Format::eR16G16B16A16Sfloat;
                    break;

                case AttachmentType::Depth:
                    image_aspect = vk::ImageAspectFlagBits::eDepth;
                    image_usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
                    image_format = context.get_supported_depth_format();
                    break;

                default:
                    ASSERT(false, "Attachment type not implemented");
                    break;
            }

            for (u32 i = 0; i < frame_count; i++)
            {
                attachment[i].curr_layout = vk::ImageLayout::eUndefined;
                attachment[i].texture =
                    create_ref<RendererImage>(image_extent, image_format, image_usage, image_aspect);
            }
        }

        // Check if output is valid
        ASSERT(attachments.contains(output_attachment_name),
               "Output attachment '" + output_attachment_name + "' is not a valid attachment");
    }

    void RenderGraph::execute()
    {
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;

        const u32 curr_frame = context.get_curr_frame_number();

        // Execute passes
        for (auto* render_pass : passes)
        {
            SCOPED_PROFILE(render_pass->get_name());

            execute_render_pass(render_pass);

            // Transition layout
            for (const auto& description : render_pass->attachment_descriptions)
            {
                if (description.stage == AttachmentStage::Output && description.type == AttachmentType::Color)
                {
                    auto& attachment = attachments[description.name];

                    vk::ImageLayout new_layout = vk::ImageLayout::eTransferSrcOptimal;

                    command_buffer.transfer_layout(attachment[curr_frame].texture->get_image(),
                                                   attachment[curr_frame].curr_layout, new_layout);
                    attachment[curr_frame].curr_layout = new_layout;
                }
            }
        }

        // Transition final image layout

        auto& attachment = attachments[output_attachment_name];
        auto& description = attachment[curr_frame].description;

        if (description.stage == AttachmentStage::Output && description.type == AttachmentType::Color)
        {
            vk::ImageLayout new_layout = vk::ImageLayout::eTransferSrcOptimal;

            command_buffer.transfer_layout(attachment[curr_frame].texture->get_image(),
                                           attachment[curr_frame].curr_layout, new_layout);
            attachment[curr_frame].curr_layout = new_layout;
        }
    }

    void RenderGraph::execute_render_pass(RenderGraphPass* render_pass)
    {
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        auto& pass = render_pass->pass;

        // @TODO: this is unlikely to be a good idea
        delete static_cast<vk::RenderingInfo*>(pass.rendering_info);
        delete static_cast<vk::RenderingAttachmentInfo*>(pass.color_attachment);
        delete static_cast<vk::RenderingAttachmentInfo*>(pass.depth_attachment);

        pass.rendering_info = nullptr;
        pass.color_attachment = nullptr;
        pass.depth_attachment = nullptr;
        // @TODO: this is unlikely to be a good idea

        const u32 curr_frame = context.get_curr_frame_number();

        // Build attachment and execute pass
        for (const auto& description : render_pass->attachment_descriptions)
        {
            auto& attachment = attachments[description.name];

            if (description.stage == AttachmentStage::Input)
            {
                vk::ImageLayout new_layout = vk::ImageLayout::eShaderReadOnlyOptimal;

                command_buffer.transfer_layout(attachment[curr_frame].texture->get_image(),
                                               attachment[curr_frame].curr_layout, new_layout);

                attachment[curr_frame].curr_layout = new_layout;
            }

            else if (description.stage == AttachmentStage::Output)
            {
                vk::ImageLayout new_layout = attachment[curr_frame].curr_layout;
                vk::AttachmentLoadOp load_op = mag_to_vk(description.state);

                switch (description.type)
                {
                    case AttachmentType::Color:
                        pass.color_attachment = new vk::RenderingAttachmentInfo(
                            attachment[curr_frame].texture->get_image_view(), vk::ImageLayout::eColorAttachmentOptimal,
                            vk::ResolveModeFlagBits::eNone, {}, {}, load_op, vk::AttachmentStoreOp::eStore,
                            mag_to_vk(pass.color_clear_value));
                        new_layout = vk::ImageLayout::eColorAttachmentOptimal;
                        break;

                    case AttachmentType::Depth:
                        pass.depth_attachment = new vk::RenderingAttachmentInfo(
                            attachment[curr_frame].texture->get_image_view(),
                            vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ResolveModeFlagBits::eNone, {}, {},
                            load_op, vk::AttachmentStoreOp::eStore,
                            vk::ClearDepthStencilValue(pass.depth_stencil_clear_value.x,
                                                       pass.depth_stencil_clear_value.y));
                        break;

                    default:
                        ASSERT(false, "Attachment type not implemented");
                        break;
                }

                // This only transitions color attachments
                if (description.type == AttachmentType::Color)
                {
                    command_buffer.transfer_layout(attachment[curr_frame].texture->get_image(),
                                                   attachment[curr_frame].curr_layout, new_layout);
                    attachment[curr_frame].curr_layout = new_layout;
                }
            }
        }

        const vk::Rect2D render_area = vk::Rect2D({}, mag_to_vk(pass.size));
        const vk::Rect2D scissor = render_area;

        // Flip the viewport along the Y axis
        const vk::Viewport viewport(0, render_area.extent.height, render_area.extent.width,
                                    -static_cast<i32>(render_area.extent.height), 0.0f, 1.0f);

        pass.rendering_info = new vk::RenderingInfo(
            {}, render_area, 1, {}, *static_cast<vk::RenderingAttachmentInfo*>(pass.color_attachment),
            static_cast<vk::RenderingAttachmentInfo*>(pass.depth_attachment), {});

        command_buffer.get_handle().setViewport(0, viewport);
        command_buffer.get_handle().setScissor(0, scissor);

        command_buffer.begin_rendering(*static_cast<vk::RenderingInfo*>(render_pass->pass.rendering_info));

        render_pass->on_render(*this);

        command_buffer.end_rendering();
    }

    RendererImage& RenderGraph::get_attachment(const str& attachment_name)
    {
        const u32 curr_frame = get_context().get_curr_frame_number();
        return *attachments[attachment_name][curr_frame].texture;
    }

    RendererImage& RenderGraph::get_output_attachment() { return get_attachment(output_attachment_name); }

    const std::vector<RenderGraphPass*>& RenderGraph::get_passes() const { return passes; }
};  // namespace mag
