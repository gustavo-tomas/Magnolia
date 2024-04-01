#include "renderer/renderer.hpp"

#include "core/logger.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    void Renderer::initialize(Window& window)
    {
        this->window = std::addressof(window);

        // Create context
        ContextCreateOptions context_options = {.window = window};
        context_options.application_name = "Magnolia";
        context_options.engine_name = "Magnolia";

        this->context.initialize(context_options);
        LOG_SUCCESS("Context initialized");

        camera.initialize({-100.0f, 5.0f, 0.0f}, {0.0f, 90.0f, 0.0f}, 60.0f, window.get_size(), 0.1f, 10000.0f);
        LOG_SUCCESS("Camera initialized");

        controller.initialize(&camera, &window);
        LOG_SUCCESS("Controller initialized");
    }

    void Renderer::shutdown()
    {
        this->context.get_device().waitIdle();

        this->controller.shutdown();
        LOG_SUCCESS("Controller destroyed");

        this->camera.shutdown();
        LOG_SUCCESS("Camera destroyed");

        this->context.shutdown();
        LOG_SUCCESS("Context destroyed");
    }

    void Renderer::update(Editor& editor, StandardRenderPass& render_pass, std::vector<Model>& models, const f32 dt)
    {
        // @TODO: maybe this shouldnt be here
        controller.update(dt);

        // Skip rendering if minimized
        if (window->is_minimized()) return;

        Frame& curr_frame = context.get_curr_frame();
        Pass& pass = render_pass.get_pass();

        this->context.begin_frame();

        // Draw calls
        render_pass.before_render(curr_frame.command_buffer);
        curr_frame.command_buffer.begin_rendering(pass);

        render_pass.render(curr_frame.command_buffer, camera, models);

        curr_frame.command_buffer.end_rendering();
        render_pass.after_render(curr_frame.command_buffer);

        // @TODO: maybe dont do this here
        editor.update(curr_frame.command_buffer, render_pass.get_target_image(), models);

        // Present

        // @TODO: testing
        static bool swap = false;
        if (window->is_key_pressed(SDLK_LSHIFT)) swap = !swap;

        if (swap)
        {
            const auto extent = render_pass.get_draw_size();
            this->context.end_frame(render_pass.get_target_image(), {extent.x, extent.y, extent.z});
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
        context.recreate_swapchain(size, vk::PresentModeKHR::eImmediate);
        const uvec2 surface_extent = uvec2(context.get_surface_extent().width, context.get_surface_extent().height);

        this->camera.set_aspect_ratio(surface_extent);
    }

    void Renderer::on_mouse_move(const ivec2& mouse_dir) { this->controller.on_mouse_move(mouse_dir); }
};  // namespace mag
