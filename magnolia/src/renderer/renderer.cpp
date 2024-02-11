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

        camera.initialize(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f), 60.0f, window.get_size(), 0.1f, 1000.0f);
        LOG_SUCCESS("Camera initialized");

        controller.initialize(&camera, &window);
        LOG_SUCCESS("Controller initialized");

        render_pass.set_camera(&camera);
    }

    void Renderer::shutdown()
    {
        this->context.get_device().waitIdle();

        this->controller.shutdown();
        LOG_SUCCESS("Controller destroyed");

        this->camera.shutdown();
        LOG_SUCCESS("Camera destroyed");

        this->render_pass.shutdown();
        LOG_SUCCESS("RenderPass destroyed");

        this->context.shutdown();
        LOG_SUCCESS("Context destroyed");
    }

    void Renderer::update(Editor& editor)
    {
        controller.update(0.016667f);

        // Skip rendering if minimized
        if (window->is_minimized()) return;

        Frame& curr_frame = context.get_curr_frame();
        Pass& pass = render_pass.get_pass();

        this->context.begin_frame();

        // @TODO: testing
        if (window->is_key_down(SDLK_UP))
            render_pass.set_render_scale(render_pass.get_render_scale() + 0.01f);

        else if (window->is_key_down(SDLK_DOWN))
            render_pass.set_render_scale(render_pass.get_render_scale() - 0.01f);
        // @TODO: testing

        // Draw calls
        render_pass.before_pass(curr_frame.command_buffer);
        curr_frame.command_buffer.begin_pass(pass);
        render_pass.render(curr_frame.command_buffer);
        curr_frame.command_buffer.end_pass(pass);
        render_pass.after_pass(curr_frame.command_buffer);

        // @TODO: maybe dont do this here
        editor.update(curr_frame.command_buffer, render_pass.get_draw_image());

        // Present

        // @TODO: testing
        static bool swap = false;
        if (window->is_key_pressed(SDLK_LSHIFT)) swap = !swap;

        if (swap)
        {
            const auto extent = render_pass.get_draw_size();
            this->context.end_frame(render_pass.get_draw_image(), {extent.x, extent.y, extent.z});
        }

        else
        {
            const auto extent = editor.get_draw_size();
            this->context.end_frame(editor.get_image(), {extent.x, extent.y, 1});
        }
        // @TODO: testing
    }

    void Renderer::on_resize(const uvec2& size)
    {
        context.get_device().waitIdle();

        // Use the surface extent after recreating the swapchain
        context.recreate_swapchain(size, vk::PresentModeKHR::eFifoRelaxed);
        const uvec2 surface_extent = uvec2(context.get_surface_extent().width, context.get_surface_extent().height);

        this->render_pass.on_resize(surface_extent);
        this->camera.set_aspect_ratio(surface_extent);
    }

    void Renderer::on_mouse_move(const ivec2& mouse_dir) { this->controller.on_mouse_move(mouse_dir); }
};  // namespace mag
