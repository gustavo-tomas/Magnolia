#pragma once

#include <map>

#include "core/application.hpp"
#include "core/types.hpp"

namespace mag
{
    // @TODO: maybe this and context Timestamp can be merged
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
            void update_profile_result(const str& name, const f64 duration, const f64 time_interval_ms)
            {
                auto& app = get_application();
                auto& window = app.get_window();

                if (!results.contains(name))
                {
                    results[name] = {};
                    results[name].frame_start = window.get_time();
                }

                results[name].duration = duration;
                results[name].accumulated += duration;
                results[name].frame_count++;

                // Update average after N ms have passed
                const f64 time_elapsed = window.get_time() - results[name].frame_start;
                if (time_elapsed >= time_interval_ms)
                {
                    results[name].average = results[name].accumulated / results[name].frame_count;
                    results[name].accumulated = 0;
                    results[name].frame_count = 0;
                    results[name].frame_start = window.get_time();
                }
            }

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
            ScopedProfiler(const str& name, const f64 time_interval_ms = 1000)
                : name(name), time_interval_ms(time_interval_ms)
            {
                auto& app = get_application();
                auto& window = app.get_window();

                start = window.get_time();
            }

            ~ScopedProfiler()
            {
                auto& app = get_application();
                auto& window = app.get_window();

                const f64 end = window.get_time();

                ProfilerManager::get().update_profile_result(name, end - start, time_interval_ms);
            }

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
