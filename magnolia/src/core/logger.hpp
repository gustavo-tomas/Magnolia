#pragma once

#include <fmt/color.h>
#include <fmt/core.h>

#include "core/types.hpp"

namespace mag
{
    enum class LogType
    {
        Error,
        Warning,
        Info,
        Success
    };

    class Logger
    {
        public:
            template <typename... Args>
            static void log(const LogType log_type, const str& format, const Args&... args)
            {
                fmt::color color = fmt::color::white;
                switch (log_type)
                {
                    case LogType::Error:
                        color = fmt::color::orange_red;
                        break;

                    case LogType::Warning:
                        color = fmt::color::yellow;
                        break;

                    case LogType::Info:
                        color = fmt::color::white;
                        break;

                    case LogType::Success:
                        color = fmt::color::spring_green;
                        break;

                    default:
                        break;
                }

                fmt::print(fmt::emphasis::bold | fg(color), format + "\n", args...);
            }
    };
};  // namespace mag

#if !defined(MAG_RELEASE)
    #define LOG_ERROR(message, ...) mag::Logger::log(mag::LogType::Error, message, ##__VA_ARGS__)
    #define LOG_WARNING(message, ...) mag::Logger::log(mag::LogType::Warning, message, ##__VA_ARGS__)
    #define LOG_INFO(message, ...) mag::Logger::log(mag::LogType::Info, message, ##__VA_ARGS__)
    #define LOG_SUCCESS(message, ...) mag::Logger::log(mag::LogType::Success, message, ##__VA_ARGS__)
#else
    #define LOG_ERROR(message, ...)
    #define LOG_WARNING(message, ...)
    #define LOG_INFO(message, ...)
    #define LOG_SUCCESS(message, ...)
#endif
