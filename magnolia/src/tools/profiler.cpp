#include "tools/profiler.hpp"

#include "core/application.hpp"

namespace mag
{
    void ProfilerManager::update_profile_result(const str& name, const f64 duration, const f64 time_interval_ms)
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

    void ProfilerManager::clear_results() { results.clear(); }

    const std::map<str, ProfileResult>& ProfilerManager::get_results() const { return results; }

    ProfilerManager& ProfilerManager::get()
    {
        static ProfilerManager instance;
        return instance;
    }

    ScopedProfiler::ScopedProfiler(const str& name, const f64 time_interval_ms)
        : name(name), time_interval_ms(time_interval_ms)
    {
        auto& app = get_application();
        auto& window = app.get_window();

        start = window.get_time();
    }

    ScopedProfiler::~ScopedProfiler()
    {
        auto& app = get_application();
        auto& window = app.get_window();

        const f64 end = window.get_time();

        ProfilerManager::get().update_profile_result(name, end - start, time_interval_ms);
    }
};  // namespace mag
