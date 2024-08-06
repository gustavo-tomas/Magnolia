#include "renderer/renderer.hpp"

#include "core/logger.hpp"
#include "renderer/type_conversions.hpp"

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

    void Renderer::begin()
    {
        context->begin_frame();
        context->begin_timestamp();  // Performance query
    }

    void Renderer::end(const Image& image, const uvec3& image_size)
    {
        // Present

        context->end_timestamp();
        context->end_frame(image, vec_to_vk_extent(image_size));

        context->calculate_timestamp();  // Calculate after command recording ended
    }

    void Renderer::on_resize(const uvec2& size)
    {
        context->get_device().waitIdle();

        context->recreate_swapchain(size);
    }
};  // namespace mag
