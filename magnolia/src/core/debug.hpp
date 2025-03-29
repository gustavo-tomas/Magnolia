#pragma once

namespace mag
{
#if MAG_CONFIG_DEBUG && (__GNUC__ || __clang__)
    #include <signal.h>
    #define DEBUG_BREAK()   \
        {                   \
            raise(SIGTRAP); \
        }
#else
    #define DEBUG_BREAK() \
        {                 \
        }
#endif
};  // namespace mag
