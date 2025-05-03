#include "resources/resource.hpp"

namespace mag
{
    namespace resource
    {
        struct ResourceSystemState
        {
        };

        static ResourceSystemState* state = nullptr;

        // Forward declaration of private functions

        b8 initialize_texture_subsystem();
        void shutdown_texture_subsystem();
        void set_on_texture_loaded_callback(const ResourceLoadedCallbackFn& callback);

        b8 initialize_font_subsystem();
        void shutdown_font_subsystem();
        void set_on_font_loaded_callback(const ResourceLoadedCallbackFn& callback);

        b8 initialize_material_subsystem();
        void shutdown_material_subsystem();
        void set_on_material_loaded_callback(const ResourceLoadedCallbackFn& callback);

        b8 initialize_model_subsystem();
        void shutdown_model_subsystem();
        void set_on_model_loaded_callback(const ResourceLoadedCallbackFn& callback);

        b8 initialize_audio_subsystem();
        void shutdown_audio_subsystem();
        void set_on_audio_loaded_callback(const ResourceLoadedCallbackFn& callback);

        b8 initialize()
        {
            state = new ResourceSystemState();

            b8 result = true;
            result = result && initialize_texture_subsystem();
            result = result && initialize_font_subsystem();
            result = result && initialize_material_subsystem();
            result = result && initialize_model_subsystem();
            result = result && initialize_audio_subsystem();

            return result;
        }

        void shutdown()
        {
            shutdown_audio_subsystem();
            shutdown_model_subsystem();
            shutdown_material_subsystem();
            shutdown_font_subsystem();
            shutdown_texture_subsystem();
            delete state;
        }

        void set_on_resource_loaded_callback(const ResourceLoadedCallbackFn& callback)
        {
            resource::set_on_texture_loaded_callback(callback);
            resource::set_on_material_loaded_callback(callback);
            resource::set_on_model_loaded_callback(callback);
            resource::set_on_font_loaded_callback(callback);
            resource::set_on_audio_loaded_callback(callback);
        }
    };  // namespace resource
};      // namespace mag
