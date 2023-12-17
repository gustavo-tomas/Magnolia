#include "core/application.hpp"
#include "core/logger.hpp"

namespace mag
{
    void Application::initialize(const str& title, const u32 width, const u32 height)
    {
        // Create the window
        window.initialize(title, {width, height});
        LOG_SUCCESS("Window initialized");
    }

    void Application::shutdown()
    {
        window.shutdown();
        LOG_SUCCESS("Window destroyed");
    }

    void Application::run()
    {
        while (window.update())
        {
        }
    }
};  // namespace mag
