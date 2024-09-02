#include "threads/job_system.hpp"

namespace mag
{
    JobQueue::~JobQueue()
    {
        std::unique_lock<std::mutex> lock(jobs_mutex);
        while (!jobs.empty())
        {
            jobs.pop();
        }
    }

    void JobQueue::push(Job job)
    {
        std::unique_lock<std::mutex> lock(jobs_mutex);
        jobs.push(job);
    }

    Job JobQueue::pop()
    {
        std::unique_lock<std::mutex> lock(jobs_mutex);
        if (jobs.empty())
        {
            return Job({}, {});
        }

        Job job = jobs.front();
        jobs.pop();

        return job;
    }

    JobSystem::JobSystem(const u32 max_number_of_threads) : running(true)
    {
        for (u32 i = 0; i < max_number_of_threads; i++)
        {
            auto worker_thread = [this]
            {
                while (running)
                {
                    Job job = job_queue.pop();

                    // Execute the job
                    if (job.execute_fn)
                    {
                        const b8 result = job.execute_fn();
                        std::lock_guard<std::mutex> lock(execute_mutex);
                        execute_result_queue.push(result);
                    }

                    // Push the callback to the callback queue
                    if (job.callback_fn)
                    {
                        std::lock_guard<std::mutex> lock(callback_mutex);
                        callback_queue.push(job.callback_fn);
                    }
                }
            };

            workers.emplace_back(worker_thread);
        }
    }

    JobSystem::~JobSystem()
    {
        running = false;

        for (auto& worker : workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

    void JobSystem::process_callbacks()
    {
        std::unique_lock<std::mutex> callback_lock(callback_mutex);
        std::unique_lock<std::mutex> execute_lock(execute_mutex);

        while (!callback_queue.empty())
        {
            auto callback = callback_queue.front();
            const b8 result = execute_result_queue.front();

            callback_queue.pop();
            execute_result_queue.pop();

            callback_lock.unlock();
            execute_lock.unlock();

            // Execute the callback on the main thread
            callback(result);

            callback_lock.lock();
            execute_lock.lock();
        }
    }

    void JobSystem::add_job(Job job) { job_queue.push(job); }
};  // namespace mag
