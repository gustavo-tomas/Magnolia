#pragma once

#include <map>

#include "core/types.hpp"

namespace mag
{
    struct ProfileResult
    {
            f64 duration;
            f64 accumulated;
            f64 average;
            f64 frame_count;
            f64 frame_start;
    };

    class ProfilerManager
    {
        public:
            void update_profile_result(const str& name, const f64 duration, const f64 time_interval_ms);
            void clear_results() { results.clear(); };

            const std::map<str, ProfileResult>& get_results() const { return results; };

            static ProfilerManager& get()
            {
                static ProfilerManager instance;
                return instance;
            }

        private:
            // Keep the results ordered
            std::map<str, ProfileResult> results = {};
    };

    class ScopedProfiler
    {
        public:
            ScopedProfiler(const str& name, const f64 time_interval_ms = 100);
            ~ScopedProfiler();

        private:
            const str name;
            f64 start, time_interval_ms;
    };
};  // namespace mag

// Don't use this macro twice in the same scope
#if defined(MAG_PROFILE_ENABLED)
    #define SCOPED_PROFILE(name, ...) mag::ScopedProfiler scoped_profiler(name, ##__VA_ARGS__)
#else
    #define SCOPED_PROFILE(name, ...)
#endif
