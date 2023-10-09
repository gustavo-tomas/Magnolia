#include "core/logger.hpp"
#include "core/window.hpp"

int main(int, char**)
{
    auto window = mag::Window();

    if (!window.initialize("Magnolia"))
    {
        LOG_ERROR("Failed to initialize window");
        return 1;
    }
    LOG_SUCCESS("Window initialized");

    window.shutdown();
    LOG_SUCCESS("Window destroyed");

    return 0;
}
