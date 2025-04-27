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

        b8 initialize_audio_subsystem();
        void shutdown_audio_subsystem();

        b8 initialize()
        {
            state = new State();

            b8 result = true;
            result = result && initialize_texture_subsystem();
            result = result && initialize_font_subsystem();

            return result;
        }

        void shutdown()
        {
            shutdown_font_subsystem();
            shutdown_texture_subsystem();
            delete state;
        }
    };  // namespace resource
};      // namespace mag
