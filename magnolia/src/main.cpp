#include "core/application.hpp"

#ifdef _WIN32
int WinMain(int argc, char* argv[])  // @TODO: moises fix pls tyty
#else
int main(int argc, char* argv[])
#endif
{
    // Ignore unused parameter warning
    (void)argc;
    (void)argv;

    mag::Application::get().run();

    return 0;
}
