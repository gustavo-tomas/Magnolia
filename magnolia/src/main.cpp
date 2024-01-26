#include "core/application.hpp"

#ifdef _WIN32
int WinMain(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    // Ignore unused parameter warning
    (void)argc;
    (void)argv;

    mag::Application application;
    application.initialize("Magnolia");
    application.run();
    application.shutdown();

    return 0;
}
