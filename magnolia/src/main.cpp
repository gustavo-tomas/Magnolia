#include "core/application.hpp"

int main(int, char**)
{
    mag::Application application;

    application.initialize("Magnolia");
    application.run();
    application.shutdown();

    return 0;
}
