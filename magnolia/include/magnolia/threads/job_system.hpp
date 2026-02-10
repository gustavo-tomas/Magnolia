#pragma once

#include <any>
#include <functional>

#include "magnolia/core/types.hpp"

// @TODO: see travis vroman video for a nice explanation and to check job type/priority
// https://www.youtube.com/watch?v=3JbLqoDubIY&list=PLv8Ddw9K0JPg1BEO-RS-0MYs423cvLVtj&index=73

namespace mag
{
    using JobGroupHandle = u32;

    struct MAG_API JobData
    {
            b8 result = false;
            std::any data;
    };

    using JobExecuteFn = std::function<JobData()>;
    using JobCallbackFn = std::function<void(const JobData&)>;

    struct MAG_API Job
    {
            Job() = default;
            Job(JobExecuteFn&& execute, JobCallbackFn&& on_execute_finished);

            JobExecuteFn execute_fn;
            JobCallbackFn callback_fn;
    };

    namespace thread
    {
        b8 initialize_job_system(const u32 max_number_of_threads);
        void shutdown_job_system();

        // Create a job group. This is useful for the caller to track pending jobs.
        MAG_API JobGroupHandle create_job_group();

        // Destroy all jobs in a group and wait for the threads that are executing jobs belonging to that group to
        // finish. This is a blocking operation.
        MAG_API void destroy_job_group(const JobGroupHandle group);

        MAG_API void add_job(const JobGroupHandle group, const Job& job);

        MAG_API void process_callbacks();
    };  // namespace thread
};  // namespace mag
