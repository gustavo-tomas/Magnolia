#pragma once

#include "core/logger.hpp"

namespace mag
{
// Assert
#if defined(MAG_ASSERTIONS_ENABLED)
    #define ASSERT(assertion, ...)                                                              \
        {                                                                                       \
            if (!(assertion))                                                                   \
            {                                                                                   \
                LOG_ERROR("Assertion failed: {0} at {1}:{2}", __VA_ARGS__, __FILE__, __LINE__); \
                std::abort();                                                                   \
            }                                                                                   \
        }
#else
    #define ASSERT(assertion, ...)        \
        {                                 \
            static_cast<void>(assertion); \
        }
#endif
};  // namespace mag
