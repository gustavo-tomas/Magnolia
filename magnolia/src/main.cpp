#include "core/types.hpp"
#include "core/logger.hpp"

int main()
{
    ASSERT(true, "Magnolia is a go!");
    LOG_SUCCESS("{0} {1}!", "Hello", "Magnolia");
}
