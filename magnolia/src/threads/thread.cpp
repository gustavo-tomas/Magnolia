#include "magnolia/threads/thread.hpp"

#include <chrono>
#include <ratio>
#include <thread>

#include "magnolia/threads/job_system.hpp"

namespace mag
{
    namespace thread
    {
        b8 initialize()
        {
            const u32 max_number_of_threads = std::thread::hardware_concurrency();
            const b8 initialized = initialize_job_system(max_number_of_threads);

            return initialized;
        }

        void shutdown() { shutdown_job_system(); }

        void sleep(const f64 ms)
        {
            std::chrono::duration<f64, std::milli> duration(ms);
            std::this_thread::sleep_for(duration);
        }
    };  // namespace thread
};  // namespace mag
