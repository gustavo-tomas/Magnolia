#include "renderer/renderer.hpp"

#include "core/logger.hpp"

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

    void Renderer::update(RenderGraph& render_graph)
    {
        this->context->begin_frame();
        this->context->begin_timestamp();  // Performance query

        render_graph.execute();

        context->end_timestamp();

        // Present

        const auto& image = render_graph.get_output_attachment();
        const auto& extent = image.get_extent();

        this->context->end_frame(image, extent);
        this->context->calculate_timestamp();  // Calculate after command recording ended
    }

    void Renderer::on_resize(const uvec2& size)
    {
        context->get_device().waitIdle();

        context->recreate_swapchain(size);
    }
};  // namespace mag
