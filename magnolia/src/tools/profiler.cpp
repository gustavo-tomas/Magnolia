#include "magnolia/tools/profiler.hpp"

#include "magnolia/platform/platform.hpp"

namespace mag
{
    void ProfilerManager::update_profile_result(const str& name, const f64 duration, const f64 time_interval_ms)
    {
        if (!results.contains(name))
        {
            results[name] = {};
            results[name].frame_start = plat::get_time();
        }

        results[name].duration = duration;
        results[name].accumulated += duration;
        results[name].frame_count++;

        // Update average after N ms have passed
        const f64 time_elapsed = plat::get_time() - results[name].frame_start;
        if (time_elapsed >= time_interval_ms)
        {
            results[name].average = results[name].accumulated / results[name].frame_count;
            results[name].accumulated = 0;
            results[name].frame_count = 0;
            results[name].frame_start = plat::get_time();
        }
    }

    void ProfilerManager::clear_results() { results.clear(); }

    ProfileResult ProfilerManager::get_result(const str& name) const
    {
        auto it = results.find(name);

        if (it != results.end())
        {
            return it->second;
        }

        return {};
    }

    void ProfilerManager::print_results() const
    {
        if (results.empty())
        {
            return;
        }

        LOG_INFO("[Profile] ------------------- Results -------------------");
        for (const auto& [name, result] : results)
        {
            LOG_INFO("[Profile] {0:.<29.29} | Average: {1:.4f}", name, result.average);
        }
    }

    ProfilerManager& ProfilerManager::get()
    {
        static ProfilerManager instance;
        return instance;
    }

    ScopedProfiler::ScopedProfiler(str name, const f64 time_interval_ms)
        : name(std::move(name)), start(plat::get_time()), time_interval_ms(time_interval_ms)
    {
    }

    ScopedProfiler::~ScopedProfiler()
    {
        const f64 end = plat::get_time();

        ProfilerManager::get().update_profile_result(name, end - start, time_interval_ms);
    }
};  // namespace mag
