#pragma once

#include "magnolia/core/logger.hpp"

namespace mag
{
// Assert
#if MAG_ASSERTIONS_ENABLED
    #define MAG_ASSERT(assertion, ...)                                                          \
        {                                                                                       \
            if (!(assertion))                                                                   \
            {                                                                                   \
                LOG_ERROR("Assertion failed: {0} at {1}:{2}", __VA_ARGS__, __FILE__, __LINE__); \
                std::abort();                                                                   \
            }                                                                                   \
        }
#else
    #define MAG_ASSERT(assertion, ...)    \
        {                                 \
            static_cast<void>(assertion); \
        }
#endif
};  // namespace mag
