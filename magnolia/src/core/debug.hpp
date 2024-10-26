#pragma once

namespace mag
{
#if defined(MAG_DEBUG) && (defined(__GNUC__) || defined(__clang__))
    #include <signal.h>
    #define DEBUG_BREAK() raise(SIGTRAP)
#else
    #define DEBUG_BREAK()
#endif
};  // namespace mag
