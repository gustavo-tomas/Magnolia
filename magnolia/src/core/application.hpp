#pragma once

#include <memory>

#include "core/event.hpp"
#include "core/file_system.hpp"
#include "core/window.hpp"
#include "physics/physics.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "resources/image_loader.hpp"
#include "resources/material.hpp"
#include "resources/material_loader.hpp"
#include "resources/model.hpp"
#include "resources/model_loader.hpp"
#include "resources/shader_loader.hpp"
#include "threads/job_system.hpp"

namespace mag
{
    // Expand app options if necessary
    struct ApplicationOptions
    {
            uvec2 size = WindowOptions::MAX_SIZE;
            ivec2 position = WindowOptions::CENTER_POS;
            str title = "Magnolia";
            str window_icon = "";
    };

    class Application
    {
        public:
            explicit Application(const ApplicationOptions& options);
            virtual ~Application();

            // The main function will call this, not the user
            void run();

            virtual void on_event(Event& e) = 0;
            virtual void on_update(const f32 dt) = 0;

            Window& get_window() { return *window; };
            Renderer& get_renderer() { return *renderer; };
            FileSystem& get_file_system() { return *file_system; };
            FileWatcher& get_file_watcher() { return *file_watcher; };
            JobSystem& get_job_system() { return *job_system; };
            ImageLoader& get_image_loader() { return *image_loader; };
            MaterialLoader& get_material_loader() { return *material_loader; };
            ModelLoader& get_model_loader() { return *model_loader; };
            TextureManager& get_texture_manager() { return *texture_loader; };
            MaterialManager& get_material_manager() { return *material_manager; };
            ModelManager& get_model_manager() { return *model_manager; };
            ShaderLoader& get_shader_loader() { return *shader_loader; };
            ShaderManager& get_shader_manager() { return *shader_manager; };
            PhysicsEngine& get_physics_engine() { return *physics_engine; };

        protected:
            // Process events from the user application
            void process_user_application_event(Event& e);

        private:
            void process_event(Event& e);
            void on_window_close(WindowCloseEvent& e);
            void on_quit(QuitEvent& e);

            unique<Window> window;
            unique<Renderer> renderer;
            unique<FileSystem> file_system;
            unique<FileWatcher> file_watcher;
            unique<JobSystem> job_system;
            unique<ImageLoader> image_loader;
            unique<MaterialLoader> material_loader;
            unique<ModelLoader> model_loader;
            unique<ShaderLoader> shader_loader;
            unique<TextureManager> texture_loader;
            unique<MaterialManager> material_manager;
            unique<ModelManager> model_manager;
            unique<ShaderManager> shader_manager;
            unique<PhysicsEngine> physics_engine;

            b8 running;
    };

    // @TODO: idk if this is thread safe but i wont use singletons <:(
    Application& get_application();

    // Defined by the client
    Application* create_application();
};  // namespace mag
