#pragma once

#include <format>
#include <print>
#include <source_location>

#include "magnolia/core/types.hpp"

namespace mag
{
    namespace log
    {
        namespace color
        {
            constexpr str Red = "\e[1;91m";
            constexpr str Green = "\e[1;92m";
            constexpr str Yellow = "\e[1;93m";
            constexpr str Blue = "\e[1;94m";
            constexpr str Purple = "\e[1;95m";
            constexpr str White = "\e[1;97m";
            constexpr str Reset = "\033[0m";
        };  // namespace color

        template <typename... Args>
        MAG_API void log_message(const str& level, const str& color, const std::source_location& loc,
                                 const std::format_string<Args...> fmt, Args&&... args)
        {
            // Remove '../' for prettier printing
            const str location = loc.file_name();
            const u64 location_start = location.find_first_not_of("../");
            const str pretty_location = location.substr(location_start);

            std::print("{}[{}:{}]\n{}", color::Blue, pretty_location, loc.line(), color::Reset);
            std::print("{}[{}] ", color, level);
            std::print(fmt, std::forward<Args>(args)...);
            std::print("\n{}", color::Reset);
        }

        template <typename... Args>
        MAG_API str get_formatted_log(const std::string_view fmt, Args&... args)
        {
            const str formatted_str = std::vformat(fmt, std::make_format_args(args...)) + "\n";

            return formatted_str;
        }
    };  // namespace log
};  // namespace mag

#if MAG_CONFIG_DEBUG
    #define LOG_ERROR(...) \
        mag::log::log_message("ERROR", mag::log::color::Red, std::source_location::current(), __VA_ARGS__)

    #define LOG_WARNING(...) \
        mag::log::log_message("WARNING", mag::log::color::Yellow, std::source_location::current(), __VA_ARGS__)

    #define LOG_INFO(...) \
        mag::log::log_message("INFO", mag::log::color::White, std::source_location::current(), __VA_ARGS__)

    #define LOG_SUCCESS(...) \
        mag::log::log_message("SUCCESS", mag::log::color::Green, std::source_location::current(), __VA_ARGS__)

#else
    #define LOG_ERROR(message, ...)
    #define LOG_WARNING(message, ...)
    #define LOG_INFO(message, ...)
    #define LOG_SUCCESS(message, ...)
#endif
