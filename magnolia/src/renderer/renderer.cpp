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

        // Validation only on debug
#if !defined(MAG_RELEASE)
        context_options.validation_layers = {"VK_LAYER_KHRONOS_validation"};
#endif

        this->context.initialize(context_options);
        LOG_SUCCESS("Context initialized");

        this->render_pass.initialize(window.get_size());
        LOG_SUCCESS("RenderPass initialized");
    }

    void Renderer::shutdown()
    {
        this->context.get_device().waitIdle();

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

        this->context.begin_frame();
        curr_frame.command_buffer.begin_pass(pass);

        // Draw calls
        render_pass.render(curr_frame.command_buffer);

        curr_frame.command_buffer.end_pass(pass);
        this->context.end_frame();
    }

    void Renderer::on_resize(const uvec2& size)
    {
        context.get_device().waitIdle();

        // @TODO: recalculate camera matrices

        // Use the surface extent after recreating the swapchain
        context.recreate_swapchain(size, vk::PresentModeKHR::eFifoRelaxed);
        const uvec2 surface_extent = uvec2(context.get_surface_extent().width, context.get_surface_extent().height);

        render_pass.on_resize(surface_extent);
    }
};  // namespace mag
