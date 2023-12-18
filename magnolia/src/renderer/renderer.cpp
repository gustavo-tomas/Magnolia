#include "renderer/renderer.hpp"

#include "core/logger.hpp"

namespace mag
{
    void Renderer::initialize(Window& window)
    {
        // Create context
        ContextCreateOptions context_options = {};
        context_options.application_name = "Magnolia";
        context_options.engine_name = "Magnolia";
        // context_options.extensions = window.get_instance_extensions();
        context_options.layers = {"VK_LAYER_KHRONOS_validation"};
        this->context.create_instance(context_options);
        LOG_SUCCESS("Instance created");
    }

    void Renderer::shutdown()
    {
        this->context.destroy();
        LOG_SUCCESS("Instance destroyed");
    }

    void Renderer::update() {}
};  // namespace mag
