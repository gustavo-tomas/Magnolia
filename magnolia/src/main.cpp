#include "core/application.hpp"
#include "core/logger.hpp"

int main(int, char**)
{
    auto application = mag::Application();

    if (!application.initialize("Magnolia", 800, 600))
    {
        LOG_ERROR("Failed to initialize application");
        return 1;
    }

    application.run();
    application.shutdown();

    return 0;
}
