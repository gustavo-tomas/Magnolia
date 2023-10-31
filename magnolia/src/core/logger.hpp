#pragma once

#include <format>
#include <iostream>

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
            static void log(const LogType log_type, const std::string_view format, const Args&... args)
            {
                str color = "";
                str reset = "\033[0m";
                switch (log_type)
                {
                    case LogType::Error:
                        color = "\033[31;1m";
                        break;

                    case LogType::Warning:
                        color = "\033[33;1m";
                        break;

                    case LogType::Info:
                        color = "\033[37;1m";
                        break;

                    case LogType::Success:
                        color = "\033[32;1m";
                        break;

                    default:
                        break;
                }

                str formatted_str = color + std::vformat(format, std::make_format_args(args...)) + reset;
                std::cout << formatted_str << "\n";
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
