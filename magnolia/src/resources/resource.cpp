#include "resources/resource.hpp"

namespace mag
{
    namespace resource
    {
        struct State
        {
        };

        static State* state = nullptr;

        b8 initialize_texture_subsystem();
        void shutdown_texture_subsystem();

        b8 initialize_font_subsystem();
        void shutdown_font_subsystem();

        b8 initialize_material_subsystem();
        void shutdown_material_subsystem();

        b8 initialize_model_subsystem();
        void shutdown_model_subsystem();

        b8 initialize_shader_subsystem();
        void shutdown_shader_subsystem();

        b8 initialize_audio_subsystem();
        void shutdown_audio_subsystem();

        b8 initialize()
        {
            state = new State();

            b8 result = true;
            result = result && initialize_texture_subsystem();
            result = result && initialize_font_subsystem();
            result = result && initialize_material_subsystem();
            result = result && initialize_model_subsystem();
            result = result && initialize_shader_subsystem();
            result = result && initialize_audio_subsystem();

            return result;
        }

        void shutdown()
        {
            shutdown_audio_subsystem();
            shutdown_shader_subsystem();
            shutdown_model_subsystem();
            shutdown_material_subsystem();
            shutdown_font_subsystem();
            shutdown_texture_subsystem();
            delete state;
        }
    };  // namespace resource
};      // namespace mag
