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
    };

    class ProfilerManager
    {
        public:
            void update_profile_result(const str& name, const ProfileResult& result) { results[name] = result; }
            void clear_results() { results.clear(); };

            const std::map<str, ProfileResult>& get_results() const { return results; };

            static ProfilerManager& get()
            {
                static ProfilerManager instance;
                return instance;
            }

        private:
            // Keep the results ordered
            std::map<str, ProfileResult> results;
    };

    class ScopedProfiler
    {
        public:
            ScopedProfiler(const str& name) : name(name)
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
                ProfileResult result = {end - start};

                ProfilerManager::get().update_profile_result(name, result);
            }

        private:
            const str name;
            f64 start;
    };
};  // namespace mag

// Don't use this macro twice in the same scope
#if defined(MAG_PROFILE_ENABLED)
    #define SCOPED_PROFILE(name) mag::ScopedProfiler scoped_profiler(name)
#else
    #define SCOPED_PROFILE(name)
#endif
