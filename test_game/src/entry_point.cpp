#include "application.hpp"

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

    // We can use a unique ptr now
    mag::unique<game::TestGame> test_game = mag::create_unique<game::TestGame>();

    test_game->run();

    return 0;
}
