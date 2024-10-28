#pragma once

#include "core/types.hpp"

namespace mag
{
    class Window;
    class Renderer;
    class PhysicsEngine;
    class FileSystem;
    class FileWatcher;
    class JobSystem;

    class ShaderManager;
    class TextureManager;
    class ModelManager;
    class MaterialManager;

    class ImageLoader;
    class MaterialLoader;
    class ModelLoader;
    class ShaderLoader;

    struct Event;
    struct QuitEvent;
    struct WindowCloseEvent;
    struct WindowResizeEvent;

    class Application
    {
        public:
            explicit Application(const str& config_file_path);
            virtual ~Application();

            // The main function will call this, not the user
            void run();

            virtual void on_event(Event& e) = 0;
            virtual void on_update(const f32 dt) = 0;

            // -1 is no limits
            void set_target_frame_rate(const f32 frame_rate);

            Window& get_window();
            Renderer& get_renderer();
            FileSystem& get_file_system();
            FileWatcher& get_file_watcher();
            JobSystem& get_job_system();
            ImageLoader& get_image_loader();
            MaterialLoader& get_material_loader();
            ModelLoader& get_model_loader();
            TextureManager& get_texture_manager();
            MaterialManager& get_material_manager();
            ModelManager& get_model_manager();
            ShaderLoader& get_shader_loader();
            ShaderManager& get_shader_manager();
            PhysicsEngine& get_physics_engine();

        protected:
            // Process events from the user application
            void process_user_application_event(Event& e);

        private:
            void process_event(Event& e);
            void on_window_close(WindowCloseEvent& e);
            void on_quit(QuitEvent& e);

            struct IMPL;
            unique<IMPL> impl;
    };

    // @TODO: idk if this is thread safe but i wont use singletons <:(
    Application& get_application();

    // Defined by the client
    Application* create_application();
};  // namespace mag
