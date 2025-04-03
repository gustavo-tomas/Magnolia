#include "core/entry_point.hpp"

#if MAG_PLATFORM_WINDOWS
    #define _main_ WinMain
#elif MAG_PLATFORM_LINUX
    #define _main_ main
#else
    #error "Undefined entry point"
#endif

int _main_(int argc, char* argv[])
{
    // Ignore unused parameter warning
    (void)argc;
    (void)argv;

    mag::Application* app = mag::create_application();

    app->run();

    delete app;

    return 0;
}
