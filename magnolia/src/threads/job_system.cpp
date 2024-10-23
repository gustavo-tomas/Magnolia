#include "threads/job_system.hpp"

#include <mutex>
#include <queue>
#include <thread>

namespace mag
{

    Job::Job(const JobExecuteFn& execute, const JobCallbackFn& on_execute_finished)
        : execute_fn(std::move(execute)), callback_fn(std::move(on_execute_finished))
    {
    }

    struct JobQueue::IMPL
    {
            IMPL() {}

            std::queue<Job> jobs;
            std::mutex jobs_mutex;
    };

    JobQueue::JobQueue() : impl(new IMPL()) {}

    JobQueue::~JobQueue()
    {
        std::unique_lock<std::mutex> lock(impl->jobs_mutex);
        while (!impl->jobs.empty())
        {
            impl->jobs.pop();
        }
    }

    void JobQueue::push(Job job)
    {
        std::unique_lock<std::mutex> lock(impl->jobs_mutex);
        impl->jobs.push(job);
    }

    Job JobQueue::pop()
    {
        std::unique_lock<std::mutex> lock(impl->jobs_mutex);
        if (impl->jobs.empty())
        {
            return Job({}, {});
        }

        Job job = impl->jobs.front();
        impl->jobs.pop();

        return job;
    }

    struct JobSystem::IMPL
    {
            IMPL() : running(true) {}

            JobQueue job_queue;
            std::vector<std::thread> workers;

            std::queue<JobCallbackFn> callback_queue;
            std::queue<b8> execute_result_queue;
            std::mutex callback_mutex;
            std::mutex execute_mutex;

            b8 running;
    };

    JobSystem::JobSystem(const u32 max_number_of_threads) : impl(new IMPL())
    {
        for (u32 i = 0; i < max_number_of_threads; i++)
        {
            auto worker_thread = [this]
            {
                while (impl->running)
                {
                    Job job = impl->job_queue.pop();

                    // Execute the job
                    if (job.execute_fn)
                    {
                        const b8 result = job.execute_fn();
                        std::lock_guard<std::mutex> lock(impl->execute_mutex);
                        impl->execute_result_queue.push(result);
                    }

                    // Push the callback to the callback queue
                    if (job.callback_fn)
                    {
                        std::lock_guard<std::mutex> lock(impl->callback_mutex);
                        impl->callback_queue.push(job.callback_fn);
                    }
                }
            };

            impl->workers.emplace_back(worker_thread);
        }
    }

    JobSystem::~JobSystem()
    {
        impl->running = false;

        for (auto& worker : impl->workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    void JobSystem::process_callbacks()
    {
        std::unique_lock<std::mutex> callback_lock(impl->callback_mutex);
        std::unique_lock<std::mutex> execute_lock(impl->execute_mutex);

        while (!impl->callback_queue.empty())
        {
            auto callback = impl->callback_queue.front();
            const b8 result = impl->execute_result_queue.front();

            impl->callback_queue.pop();
            impl->execute_result_queue.pop();

            callback_lock.unlock();
            execute_lock.unlock();

            // Execute the callback on the main thread
            callback(result);

            callback_lock.lock();
            execute_lock.lock();
        }
    }

    void JobSystem::add_job(Job job) { impl->job_queue.push(job); }
};  // namespace mag
