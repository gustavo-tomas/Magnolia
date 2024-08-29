#include "threads/job_system.hpp"

namespace mag
{
    JobQueue::JobQueue() : running(true) {}

    JobQueue::~JobQueue()
    {
        std::unique_lock<std::mutex> lock(mutex);
        while (!jobs.empty())
        {
            Job* job = jobs.front();
            jobs.pop();

            delete job;
        }
    }

    void JobQueue::push(Job* job)
    {
        std::unique_lock<std::mutex> lock(mutex);
        jobs.push(job);
        condition.notify_one();
    }

    Job* JobQueue::pop()
    {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [this] { return !jobs.empty() || !running; });

        Job* job = jobs.front();
        jobs.pop();

        return job;
    }

    void JobQueue::stop()
    {
        std::unique_lock<std::mutex> lock(mutex);
        running = false;
        condition.notify_all();
    }

    JobSystem::JobSystem(const u32 max_number_of_threads) : running(true)
    {
        for (u32 i = 0; i < max_number_of_threads; i++)
        {
            auto worker_thread = [this]
            {
                while (running)
                {
                    Job* job = job_queue.pop();

                    if (job == nullptr)
                    {
                        continue;
                    }

                    // Execute the job
                    if (job->execute_fn)
                    {
                        job->execute_fn();
                    }

                    // Push the callback to the callback queue
                    if (job->callback_fn)
                    {
                        std::lock_guard<std::mutex> lock(callback_mutex);
                        callback_queue.push(job->callback_fn);
                    }

                    delete job;
                }
            };

            workers.emplace_back(worker_thread);
        }
    }

    JobSystem::~JobSystem()
    {
        // @TODO: figure out shutdown

        running = false;
        job_queue.stop();

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
        std::unique_lock<std::mutex> lock(callback_mutex);
        while (!callback_queue.empty())
        {
            auto callback = callback_queue.front();
            callback_queue.pop();
            lock.unlock();

            // Execute the callback on the main thread
            callback();

            lock.lock();
        }
    }

    void JobSystem::add_job(Job* job) { job_queue.push(job); }
};  // namespace mag
