#include "core/application.hpp"
#include "core/logger.hpp"

namespace mag
{
    b8 Application::initialize(str title, u32 width, u32 height)
    {
        // Create the window
        window = std::make_unique<Window>();
        if (!window->initialize(title, width, height))
        {
            LOG_ERROR("Failed to initialize window");
            return false;
        }
        LOG_SUCCESS("Window initialized");

        return true;
    }

    void Application::shutdown()
    {
        // Free resources
        window->shutdown();
        LOG_SUCCESS("Window destroyed");
    }

    void Application::run()
    {
        while (!window->quit())
        {
            window->update();
        }
    }
};
