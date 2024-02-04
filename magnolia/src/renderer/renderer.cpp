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
#if defined(MAG_DEBUG)
        context_options.validation_layers = {"VK_LAYER_KHRONOS_validation"};
#endif

        this->context.initialize(context_options);
        LOG_SUCCESS("Context initialized");

        this->render_pass.initialize({context.get_surface_extent().width, context.get_surface_extent().height});
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

        // @TODO: testing
        if (window->is_key_down(SDLK_SPACE))
            render_pass.set_render_scale(render_pass.get_render_scale() + 0.01f);

        else if (window->is_key_down(SDLK_LCTRL))
            render_pass.set_render_scale(render_pass.get_render_scale() - 0.01f);
        // @TODO: testing

        // Draw calls
        render_pass.before_pass(curr_frame.command_buffer);
        curr_frame.command_buffer.begin_pass(pass);
        render_pass.render(curr_frame.command_buffer);
        curr_frame.command_buffer.end_pass(pass);
        render_pass.after_pass(curr_frame.command_buffer);

        // Present
        const auto extent = render_pass.get_draw_size();
        this->context.end_frame(render_pass.get_draw_image(), {extent.x, extent.y, extent.z});
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
