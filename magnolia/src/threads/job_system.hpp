#pragma once

#include <functional>

#include "core/types.hpp"

// @TODO: see travis vroman video for a nice explanation and to check job type/priority
// https://www.youtube.com/watch?v=3JbLqoDubIY&list=PLv8Ddw9K0JPg1BEO-RS-0MYs423cvLVtj&index=73

namespace mag
{
    typedef std::function<b8()> JobExecuteFn;
    typedef std::function<void(const b8)> JobCallbackFn;

    struct Job
    {
            Job(const JobExecuteFn& execute, const JobCallbackFn& on_execute_finished);

            const JobExecuteFn execute_fn;
            const JobCallbackFn callback_fn;
    };

    namespace thread
    {
        void add_job(Job job);
        void process_callbacks();
    };  // namespace thread
};      // namespace mag
