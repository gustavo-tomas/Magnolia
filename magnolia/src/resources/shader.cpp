#include "resources/shader.hpp"

#include <map>

#include "resources/resource.hpp"
#include "resources/resource_loader.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    namespace resource
    {
        struct State
        {
                std::map<str, ref<Shader>> shaders;
                ResourceLoadedCallbackFn on_shader_loaded;
        };

        static State* state = nullptr;

        b8 initialize_shader_subsystem()
        {
            state = new State();
            return state != nullptr;
        }

        void shutdown_shader_subsystem()
        {
            state->shaders.clear();
            delete state;
        }

        ref<Shader> get_shader(const str& name)
        {
            auto it = state->shaders.find(name);
            if (it != state->shaders.end())
            {
                return it->second;
            }

            // Create a new shader
            Shader* shader = new Shader();
            shader->loading_status = LoadingStatus::InProgress;
            state->shaders[name] = ref<Shader>(shader);

            // Temporary shader to load data into
            Shader* transfer_shader = new Shader(*shader);

            // Load in another thread
            auto execute = [name, transfer_shader]
            {
                const b8 result = resource::load(name, transfer_shader);

                if (result)
                {
                    transfer_shader->loading_status = LoadingStatus::Finished;
                }

                else
                {
                    transfer_shader->loading_status = LoadingStatus::Error;
                }

                return result;
            };

            // Callback when finished loading
            auto load_finished_callback = [shader, transfer_shader](const b8 result)
            {
                // Update the shader data
                if (result == true)
                {
                    *shader = *transfer_shader;
                    state->on_shader_loaded(shader);
                }

                // We can dispose of the temporary shader now
                delete transfer_shader;
            };

            Job load_job = Job(execute, load_finished_callback);
            thread::add_job(load_job);

            return state->shaders[name];
        }

        void set_on_shader_loaded_callback(const ResourceLoadedCallbackFn& callback)
        {
            state->on_shader_loaded = callback;
        }
    };  // namespace resource
};      // namespace mag
