#pragma once

#include "magnolia/core/logger.hpp"

namespace mag
{
// Assert
#if MAG_ASSERTIONS_ENABLED
    #define MAG_ASSERT(assertion, ...)                         \
        {                                                      \
            if (!(assertion))                                  \
            {                                                  \
                LOG_ERROR("Assertion failed: {}", #assertion); \
                LOG_ERROR(__VA_ARGS__);                        \
                std::abort();                                  \
            }                                                  \
        }
#else
    #define MAG_ASSERT(assertion, ...)    \
        {                                 \
            static_cast<void>(assertion); \
        }
#endif
};  // namespace mag
