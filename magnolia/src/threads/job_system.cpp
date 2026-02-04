#include "magnolia/threads/job_system.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "magnolia/threads/containers.hpp"

namespace mag
{
    Job::Job(JobExecuteFn&& execute, JobCallbackFn&& on_execute_finished)
        : execute_fn(std::move(execute)), callback_fn(std::move(on_execute_finished))
    {
    }

    namespace thread
    {
        struct State
        {
                Queue<Job> job_queue;
                std::vector<std::thread> workers;

                std::queue<JobCallbackFn> callback_queue;
                std::queue<JobData> execute_result_queue;
                std::mutex callback_mutex;
                std::mutex execute_mutex;
                std::mutex job_available_mutex;
                std::condition_variable job_available;

                b8 running = false;
        };

        static State* state = nullptr;

        b8 initialize_job_system(const u32 max_number_of_threads)
        {
            state = new State();

            state->running = true;

            for (u32 i = 0; i < max_number_of_threads; i++)
            {
                auto worker = []
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

                        Job job = state->job_queue.pop();

                        // Execute the job
                        if (job.execute_fn)
                        {
                            const JobData result = job.execute_fn();
                            std::lock_guard<std::mutex> lock(state->execute_mutex);
                            state->execute_result_queue.push(result);
                        }

                        // Push the callback to the callback queue
                        if (job.callback_fn)
                        {
                            std::lock_guard<std::mutex> lock(state->callback_mutex);
                            state->callback_queue.push(job.callback_fn);
                        }
                    }
                };

                state->workers.emplace_back(worker);
                state->workers.back().detach();
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
            if (!state->job_queue.empty())
            {
                state->job_available.notify_one();
            }
        }

        void add_job(const Job& job) { state->job_queue.push(job); }
    };  // namespace thread
};  // namespace mag
