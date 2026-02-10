#include "magnolia/threads/job_system.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "magnolia/threads/containers.hpp"
#include "magnolia/threads/thread.hpp"

// @TODO: the system is now more stable and won't crash when a job group is destroyed. Still the callbacks are a bit
// iffy and it might be a good idea to cancel them as well.

namespace mag
{
    Job::Job(JobExecuteFn&& execute, JobCallbackFn&& on_execute_finished)
        : execute_fn(std::move(execute)), callback_fn(std::move(on_execute_finished))
    {
    }

    namespace thread
    {
        using WorkerHandle = u32;

        struct Worker
        {
                std::thread thread;
                JobGroupHandle job_group = {};
                WorkerHandle handle = {};
                b8 busy = false;
        };

        struct State
        {
                Map<JobGroupHandle, Queue<Job>> job_queues;
                std::vector<Worker> workers;
                std::queue<JobCallbackFn> callback_queue;
                std::queue<JobData> execute_result_queue;
                std::mutex callback_mutex;
                std::mutex execute_mutex;
                std::mutex job_available_mutex;
                std::mutex grab_job_mutex;
                std::condition_variable job_available;
                JobGroupHandle current_job_handle = 0;
                WorkerHandle current_worker_handle = 0;
                b8 running = false;
                u32 max_number_of_threads = 1;
        };

        static State* state = nullptr;

        static Worker create_worker();
        static Job grab_job(const WorkerHandle& handle);

        b8 initialize_job_system(const u32 max_number_of_threads)
        {
            state = new State();

            state->max_number_of_threads = max_number_of_threads;

            state->running = true;

            for (u32 i = 0; i < max_number_of_threads; i++)
            {
                state->workers.emplace_back(create_worker());
            }

            return state != nullptr;
        }

        void shutdown_job_system()
        {
            state->running = false;

            state->job_available.notify_all();

            state->workers.clear();

            delete state;
        }

        void process_callbacks()
        {
            std::unique_lock<std::mutex> callback_lock(state->callback_mutex);
            std::unique_lock<std::mutex> execute_lock(state->execute_mutex);

            while (!state->callback_queue.empty())
            {
                auto callback = state->callback_queue.front();
                const JobData result = state->execute_result_queue.front();

                state->callback_queue.pop();
                state->execute_result_queue.pop();

                callback_lock.unlock();
                execute_lock.unlock();

                // Execute the callback on the main thread
                callback(result);

                callback_lock.lock();
                execute_lock.lock();
            }

            // If the queue is not empty, make sure to tell the workers
            for (auto& [handle, queue] : state->job_queues)
            {
                if (!queue.empty())
                {
                    state->job_available.notify_one();
                }
            }
        }

        void add_job(const JobGroupHandle group, const Job& job) { state->job_queues[group].push(job); }

        JobGroupHandle create_job_group() { return state->current_job_handle++; }

        void destroy_job_group(const JobGroupHandle group)
        {
            // Erase jobs

            auto it = state->job_queues.find(group);
            if (it == state->job_queues.end())
            {
                // A job group might be created and not submit jobs during its lifetime
                return;
            }

            state->job_queues.erase(group);

            // Wait for pending jobs to finish

            for (Worker& worker : state->workers)
            {
                if (worker.job_group != group || !worker.busy)
                {
                    continue;
                }

                while (worker.busy)
                {
                    sleep(10);
                }
            }

            // Don't forget to flush callbacks

            process_callbacks();
        }

        Worker create_worker()
        {
            const WorkerHandle handle = state->current_worker_handle++;

            Worker worker = {};
            worker.handle = handle;

            worker.thread = std::thread(
                [handle]
                {
                    while (state->running)
                    {
                        // Wait for jobs or the sad ending :(
                        {
                            std::unique_lock<std::mutex> lock(state->job_available_mutex);
                            state->job_available.wait(lock);
                            lock.unlock();
                        }

                        if (!state->running)
                        {
                            break;
                        }

                        std::unique_lock<std::mutex> lock(state->grab_job_mutex);
                        Job job = grab_job(handle);
                        state->workers[handle].busy = true;
                        lock.unlock();

                        // Execute the job
                        if (job.execute_fn)
                        {
                            const JobData result = job.execute_fn();
                            std::lock_guard<std::mutex> lock(state->execute_mutex);
                            state->workers[handle].busy = false;
                            state->execute_result_queue.push(result);
                        }

                        // Push the callback to the callback queue
                        if (job.callback_fn)
                        {
                            std::lock_guard<std::mutex> lock(state->callback_mutex);
                            state->callback_queue.push(job.callback_fn);
                        }
                    }
                });

            worker.thread.detach();

            return worker;
        }

        Job grab_job(const WorkerHandle& handle)
        {
            for (auto& [job_group_handle, queue] : state->job_queues)
            {
                if (queue.empty())
                {
                    continue;
                }

                b8 found_worker = false;
                for (Worker& worker : state->workers)
                {
                    if (worker.handle == handle)
                    {
                        worker.job_group = job_group_handle;
                        found_worker = true;
                        break;
                    }
                }

                if (!found_worker)
                {
                    return {};
                }

                return queue.pop();
            }

            return {};
        }
    };  // namespace thread
};  // namespace mag
