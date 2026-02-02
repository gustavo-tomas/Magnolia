#include "magnolia/threads/job_system.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace mag
{
    Job::Job(JobExecuteFn&& execute, JobCallbackFn&& on_execute_finished)
        : execute_fn(std::move(execute)), callback_fn(std::move(on_execute_finished))
    {
    }

    namespace thread
    {
        class JobQueue
        {
            public:
                JobQueue();
                ~JobQueue();

                void push(const Job& job);
                Job pop();
                b8 empty();

            private:
                std::queue<Job> jobs;
                std::mutex jobs_mutex;
        };

        struct State
        {
                JobQueue job_queue;
                std::vector<std::thread> workers;

                std::queue<JobCallbackFn> callback_queue;
                std::queue<b8> execute_result_queue;
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
                auto worker_thread = []
                {
                    while (state->running)
                    {
                        // Wait for jobs or the sad ending :(
                        std::unique_lock<std::mutex> lock(state->job_available_mutex);
                        state->job_available.wait(lock, [] { return !state->job_queue.empty() || !state->running; });
                        lock.unlock();

                        if (!state->running)
                        {
                            break;
                        }

                        Job job = state->job_queue.pop();

                        // Execute the job
                        if (job.execute_fn)
                        {
                            const b8 result = job.execute_fn();
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

                state->workers.emplace_back(worker_thread);
            }

            return state != nullptr;
        }

        void shutdown_job_system()
        {
            state->running = false;

            state->job_available.notify_all();

            for (auto it = state->workers.end() - 1; it >= state->workers.begin(); it--)
            {
                it->detach();
                state->workers.erase(it);
            }

            delete state;
        }

        void process_callbacks()
        {
            std::unique_lock<std::mutex> callback_lock(state->callback_mutex);
            std::unique_lock<std::mutex> execute_lock(state->execute_mutex);

            while (!state->callback_queue.empty())
            {
                auto callback = state->callback_queue.front();
                const b8 result = state->execute_result_queue.front();

                state->callback_queue.pop();
                state->execute_result_queue.pop();

                callback_lock.unlock();
                execute_lock.unlock();

                // Execute the callback on the main thread
                callback(result);

                callback_lock.lock();
                execute_lock.lock();
            }
        }

        void add_job(const Job& job) { state->job_queue.push(job); }

        JobQueue::JobQueue() = default;

        JobQueue::~JobQueue()
        {
            std::unique_lock<std::mutex> lock(jobs_mutex);
            while (!jobs.empty())
            {
                jobs.pop();
            }
        }

        void JobQueue::push(const Job& job)
        {
            std::unique_lock<std::mutex> lock(jobs_mutex);
            jobs.push(job);
            state->job_available.notify_one();
        }

        Job JobQueue::pop()
        {
            std::unique_lock<std::mutex> lock(jobs_mutex);
            if (jobs.empty())
            {
                return {{}, {}};
            }

            Job job = jobs.front();
            jobs.pop();

            return job;
        }

        b8 JobQueue::empty()
        {
            std::unique_lock<std::mutex> lock(jobs_mutex);
            return jobs.empty();
        }
    };  // namespace thread
};  // namespace mag
