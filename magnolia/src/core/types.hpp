#pragma once

#include <cstdint>
#include <memory>
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

    // Chars
    typedef char c8;
    typedef unsigned char uc8;

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

    static_assert(sizeof(c8) == 1, "Expected c8 to be 1 bytes.");
    static_assert(sizeof(uc8) == 1, "Expected uc8 to be 1 bytes.");

    static_assert(sizeof(b8) == 1, "Expected b8 to be 1 byte.");

    // Shorthands for smart pointers
    // (see Hazel: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Base.h)
    template <typename T>
    using unique = std::unique_ptr<T>;

    template <typename T>
    using ref = std::shared_ptr<T>;

    template <typename T, typename... Args>
    constexpr unique<T> create_unique(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    constexpr ref<T> create_ref(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

// Common macros
#define VEC_SIZE_BYTES(vec) (vec.empty() ? 0 : vec.size() * sizeof(vec[0])) /* Vector size in bytes */
#define MAG_TIMEOUT 1'000'000'000                                           /* 1 second in nanoseconds */
#define BIND_FN(x) std::bind(&x, this, std::placeholders::_1)               /* Shortcut to bind methods */

};  // namespace mag
