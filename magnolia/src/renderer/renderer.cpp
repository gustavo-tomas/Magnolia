#include "renderer/renderer.hpp"

#include "core/logger.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    Renderer::Renderer(Window& window) : window(window)
    {
        // Create context
        const ContextCreateOptions context_options = {.window = window};
        context = std::make_unique<Context>(context_options);
        LOG_SUCCESS("Context initialized");
    }

    Renderer::~Renderer() { this->context->get_device().waitIdle(); }

    void Renderer::update(Scene& scene, Editor& editor)
    {
        Frame& curr_frame = context->get_curr_frame();
        Camera& camera = scene.get_camera();
        ECS& ecs = scene.get_ecs();
        StandardRenderPass& render_pass = scene.get_render_pass();
        Pass& pass = render_pass.get_pass();

        this->context->begin_frame();

        // Draw calls
        render_pass.before_render();
        curr_frame.command_buffer.begin_rendering(pass);

        render_pass.render(camera, ecs);

        curr_frame.command_buffer.end_rendering();
        render_pass.after_render();

        // @TODO: maybe dont do this here
        editor.render(camera, ecs);

        // Present

        // @TODO: testing
        static b8 swap = false;
        if (window.is_key_pressed(SDLK_TAB)) swap = !swap;

        const auto& extent = swap ? render_pass.get_draw_size() : editor.get_draw_size();
        const auto& image = swap ? render_pass.get_target_image() : editor.get_image();

        this->context->end_frame(image, {extent.x, extent.y, extent.z});
        // @TODO: testing
    }

    void Renderer::on_resize(const uvec2& size)
    {
        context->get_device().waitIdle();

        context->recreate_swapchain(size);
    }
};  // namespace mag
