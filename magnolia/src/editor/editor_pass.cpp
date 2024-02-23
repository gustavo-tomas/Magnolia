#include "editor/editor_pass.hpp"

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    void EditorRenderPass::initialize(const uvec2& size)
    {
        this->draw_size = {size, 1};
        initialize_draw_image();
        this->on_resize(size);
    }

    void EditorRenderPass::shutdown()
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        draw_image.shutdown();
    }

    void EditorRenderPass::before_render(CommandBuffer& command_buffer)
    {
        auto& context = get_context();

        // Safety check
        if (draw_size.x != context.get_surface_extent().width || draw_size.y != context.get_surface_extent().height)
        {
            this->on_resize({context.get_surface_extent().width, context.get_surface_extent().height});
        }

        command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eUndefined,
                                       vk::ImageLayout::eColorAttachmentOptimal);
    }

    // Rendering is done during the update

    void EditorRenderPass::after_render(CommandBuffer& command_buffer)
    {
        // Transition the draw image and the swapchain image into their correct transfer layouts
        command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eColorAttachmentOptimal,
                                       vk::ImageLayout::eTransferSrcOptimal);
    }

    void EditorRenderPass::on_resize(const uvec2& size)
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        draw_image.shutdown();

        draw_size.x = min(size.x, context.get_surface_extent().width);
        draw_size.y = min(size.y, context.get_surface_extent().height);
        draw_size.z = 1;

        // This is not ideal but gets the job done
        this->initialize_draw_image();

        delete pass.rendering_info;
        delete pass.color_attachment;
        delete pass.depth_attachment;

        // Create attachments
        const vk::Rect2D render_area({}, {draw_size.x, draw_size.y});
        const vk::ClearValue color_clear_value({0.6f, 0.4f, 0.2f, 1.0f});

        pass.color_attachment = new vk::RenderingAttachmentInfo(
            draw_image.get_image_view(), vk::ImageLayout::eColorAttachmentOptimal, vk::ResolveModeFlagBits::eNone, {},
            {}, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, color_clear_value);

        pass.rendering_info = new vk::RenderingInfo({}, render_area, 1, {}, *pass.color_attachment, {}, {});
    }

    void EditorRenderPass::initialize_draw_image()
    {
        // The frame is rendered into this image and then copied to the swapchain
        vk::ImageUsageFlags draw_image_usage = vk::ImageUsageFlagBits::eTransferSrc |
                                               vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage |
                                               vk::ImageUsageFlagBits::eColorAttachment;

        draw_image.initialize({draw_size.x, draw_size.y, draw_size.z}, vk::Format::eR16G16B16A16Sfloat,
                              draw_image_usage, vk::ImageAspectFlagBits::eColor, 1, vk::SampleCountFlagBits::e1);
    }
};  // namespace mag
