#pragma once

#include <fmt/color.h>
#include <fmt/core.h>

#include <source_location>

#include "core/types.hpp"

namespace mag
{
    namespace log
    {
        struct LogData
        {
                fmt::color color;
        };

        MAG_API str timestamp();

        template <typename... Args>
        MAG_API void log_message(const std::source_location& location, const LogData log_data, const str& message,
                                 const Args&... args)
        {
            fmt::print(fmt::emphasis::bold | fg(fmt::color::violet), "[{0}]", timestamp());

            fmt::print(fmt::emphasis::bold | fg(fmt::color::royal_blue), "[{0}:{1}]: ", location.file_name(),
                       location.line());

            fmt::print(fmt::emphasis::bold | fg(log_data.color), message + "\n", args...);
        }
    };  // namespace log
};      // namespace mag

#if MAG_CONFIG_DEBUG
    #define LOG_ERROR(message, ...)                                                      \
        mag::log::log_message(std::source_location::current(), {fmt::color::orange_red}, \
                              message __VA_OPT__(, ) __VA_ARGS__)

    #define LOG_WARNING(message, ...) \
        mag::log::log_message(std::source_location::current(), {fmt::color::yellow}, message __VA_OPT__(, ) __VA_ARGS__)

    #define LOG_INFO(message, ...) \
        mag::log::log_message(std::source_location::current(), {fmt::color::white}, message __VA_OPT__(, ) __VA_ARGS__)

    #define LOG_SUCCESS(message, ...)                                                      \
        mag::log::log_message(std::source_location::current(), {fmt::color::spring_green}, \
                              message __VA_OPT__(, ) __VA_ARGS__)
#else
    #define LOG_ERROR(message, ...)
    #define LOG_WARNING(message, ...)
    #define LOG_INFO(message, ...)
    #define LOG_SUCCESS(message, ...)
#endif
