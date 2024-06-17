#include "renderer/renderer.hpp"

#include "core/application.hpp"
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

    void Renderer::update(BaseScene& scene, Editor& editor)
    {
        Frame& curr_frame = context->get_curr_frame();
        CommandBuffer& command_buffer = curr_frame.command_buffer;
        Camera& camera = scene.get_camera();
        ECS& ecs = scene.get_ecs();
        StandardRenderPass& render_pass = scene.get_render_pass();
        Pass& pass = render_pass.get_pass();

        statistics = {};

        // @TODO: dont do this here
        auto& app = get_application();
        auto& physics_engine = app.get_physics_engine();

        physics_debug_lines.reset();
        physics_debug_lines = nullptr;
        auto& debug_lines = physics_engine.get_line_list();
        if (!debug_lines.starts.empty())
        {
            physics_debug_lines =
                std::make_unique<Line>("DebugLines", debug_lines.starts, debug_lines.ends, debug_lines.colors);
        }
        // @TODO: dont do this here

        this->context->begin_frame();
        this->context->begin_timestamp();  // Performance query

        // Draw calls
        render_pass.before_render();
        command_buffer.begin_rendering(pass);

        render_pass.render(camera, ecs);

        command_buffer.end_rendering();
        render_pass.after_render();

        context->end_timestamp();

        statistics.draw_calls += pass.statistics.draw_calls;
        statistics.rendered_triangles += pass.statistics.rendered_triangles;

        // @TODO: maybe dont do this here
        editor.render(camera, ecs);

        // Present

        // @TODO: testing
        static b8 swap = false;
        if (window.is_key_pressed(Key::Tab)) swap = !swap;

        const auto& extent = swap ? render_pass.get_draw_size() : editor.get_draw_size();
        const auto& image = swap ? render_pass.get_target_image() : editor.get_image();

        this->context->end_frame(image, {extent.x, extent.y, extent.z});
        this->context->calculate_timestamp();  // Calculate after command recording ended
        // @TODO: testing
    }

    void Renderer::on_resize(const uvec2& size)
    {
        context->get_device().waitIdle();

        context->recreate_swapchain(size);
    }
};  // namespace mag
