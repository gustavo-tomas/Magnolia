#include "magnolia/platform/platform.hpp"

#include <chrono>

namespace mag
{
    namespace plat
    {
        struct State
        {
                std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
        };

        static State* state = nullptr;

        b8 initialize()
        {
            state = new State();

            return state != nullptr;
        }

        void shutdown() { delete state; }

        f64 get_time()
        {
            // Ms since start
            const std::chrono::time_point current_time = std::chrono::system_clock::now();
            const std::chrono::duration<f64> elapsed_seconds = current_time - state->start_time;

            return elapsed_seconds.count() * 1000.0;
        }
    };  // namespace plat
};  // namespace mag
