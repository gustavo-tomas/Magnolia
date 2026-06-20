#include "magnolia/threads/thread.hpp"

#include <chrono>
#include <ratio>
#include <thread>

#include "magnolia/threads/job_system.hpp"
#include "magnolia/threads/process_manager.hpp"

namespace mag
{
    namespace thread
    {
        b8 initialize()
        {
            const u32 max_number_of_threads = std::thread::hardware_concurrency();

            b8 initialized = false;
            initialized |= initialize_job_system(max_number_of_threads);
            initialized |= initialize_process_manager();

            return initialized;
        }

        void shutdown()
        {
            shutdown_process_manager();
            shutdown_job_system();
        }

        u32 get_core_count() { return std::thread::hardware_concurrency(); }

        void sleep(const f64 ms)
        {
            const std::chrono::duration<f64, std::milli> duration(ms);
            std::this_thread::sleep_for(duration);
        }
    };  // namespace thread
};  // namespace mag
