#pragma once

#include <functional>

#include "magnolia/core/types.hpp"

// @TODO: see travis vroman video for a nice explanation and to check job type/priority
// https://www.youtube.com/watch?v=3JbLqoDubIY&list=PLv8Ddw9K0JPg1BEO-RS-0MYs423cvLVtj&index=73

namespace mag
{
    using JobExecuteFn = std::function<b8()>;
    using JobCallbackFn = std::function<void(const b8)>;

    struct MAG_API Job
    {
            Job(JobExecuteFn&& execute, JobCallbackFn&& on_execute_finished);

            const JobExecuteFn execute_fn;
            const JobCallbackFn callback_fn;
    };

    namespace thread
    {
        b8 initialize_job_system(const u32 max_number_of_threads);
        void shutdown_job_system();

        MAG_API void add_job(const Job& job);
        MAG_API void process_callbacks();
    };  // namespace thread
};  // namespace mag
