#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/gfx/types.hpp"
#include "magnolia/platform/window.hpp"
#include "magnolia/resources/resource.hpp"

namespace mag
{
    struct Event;
    struct QuitEvent;
    struct WindowCloseEvent;
    struct WindowResizeEvent;
    struct IResource;

    struct ApplicationOptions
    {
            window::WindowOptions window_options = {};
            gfx::GfxOptions gfx_options = {};

            f32 target_frame_rate = -1;
    };

    class MAG_API Application
    {
        public:
            explicit Application(const ApplicationOptions& options);
            virtual ~Application();

            // The main function will call this, not the user
            void run();

            virtual void on_event(const Event& e) = 0;
            virtual void on_update(const f32 dt) = 0;

            // -1 is no limits
            void set_target_frame_rate(const f32 frame_rate);

        protected:
            void process_event(const Event& e);

            void set_on_resource_loaded_callback(const ResourceLoadedCallbackFn& callback);

        private:
            void on_window_close(const WindowCloseEvent& e);
            void on_quit(const QuitEvent& e);
            void on_resource_loaded(const IResource* resource);

            b8 running;
            f32 target_frame_rate;
            ResourceLoadedCallbackFn on_resource_loaded_user_callback = nullptr;
    };

    // Defined by the client
    MAG_API Application* create_application();
};  // namespace mag
