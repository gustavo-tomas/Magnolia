#pragma once

#include "core/application.hpp"

extern mag::Application* mag::create_application();

#ifdef _WIN32
int WinMain(int argc, char* argv[])  // @TODO: moises fix pls tyty
#else
int main(int argc, char* argv[])
#endif
{
    // Ignore unused parameter warning
    (void)argc;
    (void)argv;

    auto app = mag::create_application();

    app->run();

    delete app;

    return 0;
}
