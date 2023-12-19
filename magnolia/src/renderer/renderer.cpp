#include "renderer/renderer.hpp"

#include "core/logger.hpp"

namespace mag
{
    void Renderer::initialize(Window& window)
    {
        // Create context
        ContextCreateOptions context_options = {.window = window};
        context_options.application_name = "Magnolia";
        context_options.engine_name = "Magnolia";
        context_options.validation_layers = {"VK_LAYER_KHRONOS_validation"};

        this->context.initialize(context_options);
        LOG_SUCCESS("Instance created");
    }

    void Renderer::shutdown()
    {
        this->context.shutdown();
        LOG_SUCCESS("Instance destroyed");
    }

    void Renderer::update() {}

    void Renderer::resize(const uvec2& size) { context.recreate_swapchain(size, vk::PresentModeKHR::eImmediate); }
};  // namespace mag
