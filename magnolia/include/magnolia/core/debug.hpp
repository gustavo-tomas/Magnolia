#pragma once

#include <csignal>

namespace mag
{
#if MAG_CONFIG_DEBUG && (__GNUC__ || __clang__)
    #define DEBUG_BREAK()         \
        {                         \
            (void)raise(SIGTRAP); \
        }
#else
    #define DEBUG_BREAK() \
        {                 \
        }
#endif
};  // namespace mag
