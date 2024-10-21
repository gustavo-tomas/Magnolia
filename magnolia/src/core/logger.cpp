#include "core/logger.hpp"

namespace mag
{
    str const Logger::timestamp()
    {
        const auto current_time = std::chrono::system_clock::now();
        const auto current_time_t = std::chrono::system_clock::to_time_t(current_time);
        const std::tm* time_info = std::localtime(&current_time_t);

        // Get the formatted time string from the string stream
        std::ostringstream oss;
        oss << std::put_time(time_info, "%H:%M:%S");
        const str formatted_time = oss.str();

        return formatted_time;
    }
};  // namespace mag
