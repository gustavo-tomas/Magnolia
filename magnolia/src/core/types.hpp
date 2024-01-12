#pragma once

#include <cstdint>
#include <string>

namespace mag
{
    // Unsigned integers
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    // Integers
    typedef int8_t i8;
    typedef int16_t i16;
    typedef int32_t i32;
    typedef int64_t i64;

    // Floats
    typedef float f32;
    typedef double f64;

    // Bool
    typedef bool b8;

    // Strings
    typedef std::string str;

    // Assert sizes
    static_assert(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
    static_assert(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
    static_assert(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
    static_assert(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

    static_assert(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
    static_assert(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
    static_assert(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
    static_assert(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

    static_assert(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
    static_assert(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

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
    #define ASSERT(assertion, ...)
#endif

// Common macros
#define VECSIZE(vec) static_cast<u32>(vec.size())
};  // namespace mag
