#include "renderer/renderer.hpp"

#include "core/logger.hpp"

namespace mag
{
    void Renderer::initialize(Window& window)
    {
        this->window = std::addressof(window);

        // Create context
        ContextCreateOptions context_options = {.window = window};
        context_options.application_name = "Magnolia";
        context_options.engine_name = "Magnolia";
        context_options.validation_layers = {"VK_LAYER_KHRONOS_validation"};

        this->context.initialize(context_options);
        LOG_SUCCESS("Context initialized");

        this->render_pass.initialize(draw_image_resolution);
        LOG_SUCCESS("RenderPass initialized");

        // The frame is rendered into this image and then copied to the swapchain
        vk::ImageUsageFlags draw_image_usage = {};
        draw_image_usage |= vk::ImageUsageFlagBits::eTransferSrc;
        draw_image_usage |= vk::ImageUsageFlagBits::eTransferDst;
        draw_image_usage |= vk::ImageUsageFlagBits::eStorage;
        draw_image_usage |= vk::ImageUsageFlagBits::eColorAttachment;
        draw_image.initialize({draw_image_resolution.x, draw_image_resolution.y, 1}, vk::Format::eR16G16B16A16Sfloat,
                              draw_image_usage, vk::ImageAspectFlagBits::eColor, 1, vk::SampleCountFlagBits::e1);
        LOG_SUCCESS("Draw image initialized");
    }

    void Renderer::shutdown()
    {
        this->context.get_device().waitIdle();

        this->draw_image.shutdown();
        LOG_SUCCESS("Draw image destroyed");

        this->render_pass.shutdown();
        LOG_SUCCESS("RenderPass destroyed");

        this->context.shutdown();
        LOG_SUCCESS("Context destroyed");
    }

    void Renderer::update()
    {
        // Skip rendering if minimized
        if (window->is_minimized()) return;

        Frame& curr_frame = context.get_curr_frame();
        Pass& pass = render_pass.get_pass();

        // @TODO: improve resize handling

        draw_extent.width = min(context.get_surface_extent().width, draw_image.get_extent().width) * render_scale;
        draw_extent.height = min(context.get_surface_extent().height, draw_image.get_extent().height) * render_scale;
        draw_extent.depth = 1;

        if (!this->context.begin_frame()) return;

        // curr_frame.command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eUndefined,
        //                                           vk::ImageLayout::eColorAttachmentOptimal);

        // // Draw calls
        // curr_frame.command_buffer.begin_pass(pass);
        // render_pass.render(curr_frame.command_buffer);
        // curr_frame.command_buffer.end_pass(pass);
        curr_frame.command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eUndefined,
                                                  vk::ImageLayout::eGeneral);

        // Draw calls
        vk::ImageSubresourceRange clearRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        curr_frame.command_buffer.get_handle().clearColorImage(draw_image.get_image(), vk::ImageLayout::eGeneral,
                                                               {1.0f, 0.0f, 1.0f, 1.0f}, clearRange);

        // curr_frame.command_buffer.begin_pass(pass);
        // render_pass.render(curr_frame.command_buffer);
        // curr_frame.command_buffer.end_pass(pass);

        // Transition the draw image and the swapchain image into their correct transfer layouts
        curr_frame.command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eGeneral,
                                                  vk::ImageLayout::eTransferSrcOptimal);

        // // Transition the draw image and the swapchain image into their correct transfer layouts
        // curr_frame.command_buffer.transfer_layout(draw_image.get_image(), vk::ImageLayout::eColorAttachmentOptimal,
        //                                           vk::ImageLayout::eTransferSrcOptimal);

        curr_frame.command_buffer.transfer_layout(context.get_swapchain_images()[context.get_swapchain_image_index()],
                                                  vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

        // Copy from the draw image to the swapchain
        curr_frame.command_buffer.copy_image_to_image(
            draw_image.get_image(), draw_extent, context.get_swapchain_images()[context.get_swapchain_image_index()],
            vk::Extent3D(context.get_surface_extent(), 1));

        // Set swapchain image layout to Present so we can show it on the screen
        curr_frame.command_buffer.transfer_layout(context.get_swapchain_images()[context.get_swapchain_image_index()],
                                                  vk::ImageLayout::eTransferDstOptimal,
                                                  vk::ImageLayout::ePresentSrcKHR);

        if (!this->context.end_frame()) return;
    }

    void Renderer::on_resize(const uvec2& size)
    {
        context.get_device().waitIdle();

        // @TODO: recalculate camera matrices

        // Use the surface extent after recreating the swapchain
        context.recreate_swapchain(size, vk::PresentModeKHR::eFifoRelaxed);
        const uvec2 surface_extent = uvec2(context.get_surface_extent().width, context.get_surface_extent().height);

        draw_extent.width = min(surface_extent.x, draw_image.get_extent().width) * render_scale;
        draw_extent.height = min(surface_extent.y, draw_image.get_extent().height) * render_scale;
        draw_extent.depth = 1;

        render_pass.on_resize(uvec2(draw_extent.width, draw_extent.height));
    }
};  // namespace mag
