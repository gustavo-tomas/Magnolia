#pragma once

#include <unordered_map>

#include "magnolia/core/types.hpp"

namespace mag
{
    struct MAG_API ProfileResult
    {
            f64 duration;
            f64 accumulated;
            f64 average;
            f64 frame_count;
            f64 frame_start;
    };

    class MAG_API ProfilerManager
    {
        public:
            void update_profile_result(const str& name, f64 duration, f64 time_interval_ms);
            void clear_results();

            ProfileResult get_result(const str& name) const;

            static ProfilerManager& get();

        private:
            // Keep the results ordered
            std::unordered_map<str, ProfileResult> results;
    };

    class MAG_API ScopedProfiler
    {
        public:
            explicit ScopedProfiler(str name, f64 time_interval_ms = 100);
            ~ScopedProfiler();

        private:
            str name;
            f64 start, time_interval_ms;
    };

// @NOTE: Don't use this macro with the same name twice in the same scope
#if MAG_PROFILE_ENABLED
    #define MAG_SCOPED_PROFILE(name, ...) ScopedProfiler scoped_profiler(name, ##__VA_ARGS__)
#else
    #define MAG_SCOPED_PROFILE(name, ...)
#endif
};  // namespace mag
