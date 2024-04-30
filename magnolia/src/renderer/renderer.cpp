#include "renderer/renderer.hpp"

#include "core/logger.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    Renderer::Renderer(Window& window) : window(window)
    {
        // Create context
        ContextCreateOptions context_options = {.window = window};
        context_options.application_name = "Magnolia";
        context_options.engine_name = "Magnolia";

        this->context.initialize(context_options);
        LOG_SUCCESS("Context initialized");
    }

    Renderer::~Renderer()
    {
        this->context.get_device().waitIdle();

        this->context.shutdown();
        LOG_SUCCESS("Context destroyed");
    }

    void Renderer::update(const Camera& camera, Editor& editor, StandardRenderPass& render_pass,
                          std::vector<Model>& models)
    {
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
        editor.render(curr_frame.command_buffer, models, camera);

        // Present

        // @TODO: testing
        static bool swap = false;
        if (window.is_key_pressed(SDLK_TAB)) swap = !swap;

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

        // @TODO: hardcoded present mode
        context.recreate_swapchain(size, vk::PresentModeKHR::eImmediate);
    }
};  // namespace mag
