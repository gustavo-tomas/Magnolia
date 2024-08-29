#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "core/types.hpp"

// @TODO: see travis vroman video for a nice explanation and to check job type/priority
// https://www.youtube.com/watch?v=3JbLqoDubIY&list=PLv8Ddw9K0JPg1BEO-RS-0MYs423cvLVtj&index=73

namespace mag
{
    // @TODO: might be a good ideia to store the result of the execute fn and send it to the callback
    typedef std::function<void()> JobExecuteFn;
    typedef std::function<void()> JobCallbackFn;

    struct Job
    {
            Job(const JobExecuteFn& execute, const JobCallbackFn& on_execute_finished)
                : execute_fn(std::move(execute)), callback_fn(std::move(on_execute_finished)){};

            const JobExecuteFn execute_fn;
            const JobCallbackFn callback_fn;
    };

    class JobQueue
    {
        public:
            JobQueue();
            ~JobQueue();

            void push(Job* job);
            Job* pop();
            void stop();

        private:
            std::queue<Job*> jobs;
            std::mutex mutex;
            std::condition_variable condition;

            b8 running;
    };

    class JobSystem
    {
        public:
            JobSystem(const u32 max_number_of_threads);
            ~JobSystem();

            void add_job(Job* job);
            void process_callbacks();

        private:
            JobQueue job_queue;
            std::vector<std::thread> workers;

            std::queue<JobCallbackFn> callback_queue;
            std::mutex callback_mutex;

            b8 running;
    };
};  // namespace mag
