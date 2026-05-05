#pragma once

#include <climits>
#include <cstdint>
#include <memory>

// Unsigned integers
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// Integers
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// Floats
using f32 = float;
using f64 = double;

// Chars
using c8 = char;
using uc8 = unsigned char;

// Bool
using b8 = bool;

// Strings
using str = std::string;

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

namespace mag
{
    // Constants
    constexpr u32 Max_U32 = UINT_MAX;
    constexpr i32 Max_I32 = INT_MAX;
    constexpr u32 Invalid_ID = UINT_MAX;
    constexpr u64 Timeout = ULONG_LONG_MAX;

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

    // Bitwise operations

    // Define a template for enabling bit operations on enum classes
    template <typename Enum>
    struct EnableBitMaskOperators
    {
            static constexpr b8 enable = false;
    };

    // Bitwise operators that work with any enum class marked with EnableBitMaskOperators
    template <typename Enum>
    Enum operator|(Enum lhs, Enum rhs)
        requires EnableBitMaskOperators<Enum>::enable
    {
        using underlying = std::underlying_type_t<Enum>;
        return static_cast<Enum>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
    }

    template <typename Enum>
    Enum operator&(Enum lhs, Enum rhs)
        requires EnableBitMaskOperators<Enum>::enable
    {
        using underlying = std::underlying_type_t<Enum>;
        return static_cast<Enum>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
    }

    template <typename Enum>
    Enum operator^(Enum lhs, Enum rhs)
        requires EnableBitMaskOperators<Enum>::enable
    {
        using underlying = std::underlying_type_t<Enum>;
        return static_cast<Enum>(static_cast<underlying>(lhs) ^ static_cast<underlying>(rhs));
    }

    template <typename Enum>
    Enum operator~(Enum e)
        requires EnableBitMaskOperators<Enum>::enable
    {
        using underlying = std::underlying_type_t<Enum>;
        return static_cast<Enum>(~static_cast<underlying>(e));
    }

    template <typename Enum>
    Enum& operator|=(Enum& lhs, Enum rhs)
        requires EnableBitMaskOperators<Enum>::enable
    {
        lhs = lhs | rhs;
        return lhs;
    }

    template <typename Enum>
    Enum& operator&=(Enum& lhs, Enum rhs)
        requires EnableBitMaskOperators<Enum>::enable
    {
        lhs = lhs & rhs;
        return lhs;
    }

    template <typename Enum>
    Enum& operator^=(Enum& lhs, Enum rhs)
        requires EnableBitMaskOperators<Enum>::enable
    {
        lhs = lhs ^ rhs;
        return lhs;
    }

// Common macros
#define VEC_SIZE_BYTES(vec) ((vec).empty() ? 0 : (vec).size() * sizeof((vec)[0])) /* Vector size in bytes */

// Define a macro to make enum bitmask-ready
#define ENABLE_BITMASK_OPERATORS(x)            \
    template <>                                \
    struct mag::EnableBitMaskOperators<x>      \
    {                                          \
            static constexpr b8 enable = true; \
    };

#define IS_BIT_SET(x, enum) (((x) & enum) == enum)

// Platform

// Windows
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
    #error "Windows platform is not implemented"
    #ifndef _WIN64
        #error "Windows platform is not 64-bit"
    #endif

// Linux
#elif defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
    #define MAG_PLATFORM_LINUX 1

// Unknown
#else
    #error "Unknown platform"
#endif

    // DLL imports and exports

#ifdef MAG_BUILD_SHARED
    #if MAG_PLATFORM_LINUX
        #define MAG_API __attribute__((visibility("default")))
    #else
        #error "Undefined DLL configuration"
    #endif
#else
    #if MAG_PLATFORM_LINUX
        #define MAG_API
    #else
        #error "Undefined DLL configuration"
    #endif
#endif

// Paths and build configurations

// @TODO: idk if defining these macros is the best solution, but it'll keep things simple for now
#if MAG_PLATFORM_LINUX
    #define MAG_BUILD_DIR_PLATFORM "linux/"
#elif MAG_PLATFORM_WINDOWS
    #define MAG_BUILD_DIR_PLATFORM "windows/"
#endif

#if MAG_CONFIG_DEBUG
    #define MAG_BUILD_DIR_CONFIG "debug/"
#elif MAG_CONFIG_RELEASE
    #define MAG_BUILD_DIR_CONFIG "release/"
#endif

#define MAG_BUILD_DIR_BIN "build/" MAG_BUILD_DIR_PLATFORM MAG_BUILD_DIR_CONFIG "bin/"
#define MAG_BUILD_DIR_SHADERS MAG_BUILD_DIR_BIN "shaders/"
#define MAG_BUILD_DIR_SCRIPTS MAG_BUILD_DIR_BIN "scripts/"

// Returns the build path for a shader file
#define MAG_BUILD_SHADER_NAME(name) (MAG_BUILD_DIR_SHADERS + str(name) + ".spv")

// Returns the build path for a script file
#if MAG_PLATFORM_LINUX
    #define MAG_BUILD_SCRIPT_NAME(name) (MAG_BUILD_DIR_SCRIPTS + str(name) + ".so")
#elif MAG_PLATFORM_WINDOWS
    #define MAG_BUILD_SCRIPT_NAME(name) (MAG_BUILD_DIR_SCRIPTS + str(name) + ".dll")
#endif

    // Ext paths

#define MAG_EXT_DIR "ext/" MAG_BUILD_DIR_PLATFORM

#if MAG_PLATFORM_LINUX
    #define MAG_EXT_GLSLC MAG_EXT_DIR "glslc"
#else
    #error "Unsupported glslc"
#endif
};  // namespace mag
