#include "renderer/render_graph.hpp"

#include "core/types.hpp"
#include "renderer/context.hpp"
#include "renderer/render_graph_conversions.hpp"
#include "renderer/type_conversions.hpp"

namespace mag
{
    RenderGraphPass::RenderGraphPass(const str& name, const uvec2& size) : name(name), size(size) {}

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

    // Render graph
    // -----------------------------------------------------------------------------------------------------------------

    RenderGraph::~RenderGraph()
    {
        for (auto* pass : passes)
        {
            delete pass;
        }

        for (auto& [attachment_name, attachment] : attachments)
        {
            attachment.texture.shutdown();
        }
    }

    void RenderGraph::add_pass(RenderGraphPass* render_pass)
    {
        for (const auto& attachment_desc : render_pass->attachment_descriptions)
        {
            if (!attachments.contains(attachment_desc.name))
            {
                attachments[attachment_desc.name] = {};
                attachments[attachment_desc.name].description = attachment_desc;
            }
        }

        passes.push_back(render_pass);
    }

    void RenderGraph::set_output_attachment(const str& attachment_name) { output_attachment_name = attachment_name; }

    void RenderGraph::build()
    {
        auto& context = get_context();

        // Build attachments
        for (auto& [attachment_name, attachment] : attachments)
        {
            const auto& description = attachment.description;

            vk::ImageAspectFlags image_aspect = {};
            vk::ImageUsageFlags image_usage = {};
            vk::Format image_format = {};
            vk::Extent3D image_extent = vk::Extent3D(vec_to_vk_extent(description.size), 1);

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

            attachment.curr_layout = vk::ImageLayout::eUndefined;
            attachment.texture.initialize(image_extent, image_format, image_usage, image_aspect);
        }

        // Check if output is valid
        ASSERT(attachments.contains(output_attachment_name),
               "Output attachment '" + output_attachment_name + "' is not a valid attachment");
    }

    void RenderGraph::execute()
    {
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;

        // Execute passes
        for (auto* render_pass : passes)
        {
            execute_render_pass(render_pass);

            // Transition layout
            for (const auto& description : render_pass->attachment_descriptions)
            {
                auto& attachment = attachments[description.name];

                if (description.stage == AttachmentStage::Output && description.type == AttachmentType::Color)
                {
                    vk::ImageLayout new_layout = vk::ImageLayout::eTransferSrcOptimal;

                    command_buffer.transfer_layout(attachment.texture.get_image(), attachment.curr_layout, new_layout);
                    attachment.curr_layout = new_layout;
                }
            }
        }

        // Transition final image layout

        auto& attachment = attachments[output_attachment_name];
        auto& description = attachment.description;

        if (description.stage == AttachmentStage::Output && description.type == AttachmentType::Color)
        {
            vk::ImageLayout new_layout = vk::ImageLayout::eTransferSrcOptimal;

            command_buffer.transfer_layout(attachment.texture.get_image(), attachment.curr_layout, new_layout);
            attachment.curr_layout = new_layout;
        }
    }

    void RenderGraph::execute_render_pass(RenderGraphPass* render_pass)
    {
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        auto& pass = render_pass->pass;

        // Build attachment and execute pass
        for (const auto& description : render_pass->attachment_descriptions)
        {
            auto& attachment = attachments[description.name];

            if (description.stage == AttachmentStage::Input)
            {
                vk::ImageLayout new_layout = vk::ImageLayout::eShaderReadOnlyOptimal;

                command_buffer.transfer_layout(attachment.texture.get_image(), attachment.curr_layout, new_layout);

                attachment.curr_layout = new_layout;
            }

            else if (description.stage == AttachmentStage::Output)
            {
                vk::ImageLayout new_layout = attachment.curr_layout;
                vk::AttachmentLoadOp load_op = mag_attachment_state_to_vk(description.state);

                switch (description.type)
                {
                    case AttachmentType::Color:
                        pass.color_attachment = vk::RenderingAttachmentInfo(
                            attachment.texture.get_image_view(), vk::ImageLayout::eColorAttachmentOptimal,
                            vk::ResolveModeFlagBits::eNone, {}, {}, load_op, vk::AttachmentStoreOp::eStore,
                            pass.color_clear_value);
                        new_layout = vk::ImageLayout::eColorAttachmentOptimal;
                        break;

                    case AttachmentType::Depth:
                        pass.depth_attachment = vk::RenderingAttachmentInfo(
                            attachment.texture.get_image_view(), vk::ImageLayout::eDepthStencilAttachmentOptimal,
                            vk::ResolveModeFlagBits::eNone, {}, {}, load_op, vk::AttachmentStoreOp::eStore,
                            {pass.depth_clear_value});
                        break;

                    default:
                        ASSERT(false, "Attachment type not implemented");
                        break;
                }

                // This only transitions color attachments
                if (description.type == AttachmentType::Color)
                {
                    command_buffer.transfer_layout(attachment.texture.get_image(), attachment.curr_layout, new_layout);
                    attachment.curr_layout = new_layout;
                }
            }
        }

        const vk::Rect2D render_area = vk::Rect2D({}, vec_to_vk_extent(pass.size));
        const vk::Viewport viewport(0, 0, render_area.extent.width, render_area.extent.height, 0.0f, 1.0f);
        const vk::Rect2D scissor = render_area;

        pass.rendering_info =
            vk::RenderingInfo({}, render_area, 1, {}, pass.color_attachment, &pass.depth_attachment, {});

        command_buffer.get_handle().setViewport(0, viewport);
        command_buffer.get_handle().setScissor(0, scissor);

        command_buffer.begin_rendering(render_pass->pass.rendering_info);

        render_pass->on_render(*this);

        command_buffer.end_rendering();
    }
};  // namespace mag
