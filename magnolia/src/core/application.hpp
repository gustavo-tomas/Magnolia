#pragma once

#include <memory>

#include "core/event.hpp"
#include "core/window.hpp"
#include "editor/editor.hpp"
#include "physics/physics.hpp"
#include "renderer/image.hpp"
#include "renderer/material.hpp"
#include "renderer/model.hpp"
#include "renderer/renderer.hpp"
#include "renderer/shader.hpp"
#include "scene/scene.hpp"

namespace mag
{
    // Expand app options if necessary
    struct ApplicationOptions
    {
            uvec2 size = WindowOptions::MAX_SIZE;
            ivec2 position = WindowOptions::CENTER_POS;
            str title = "Magnolia";
    };

    class Application
    {
        public:
            explicit Application(const ApplicationOptions& options);
            virtual ~Application() = default;

            void run();
            void on_event(Event& e);

            void set_active_scene(BaseScene* scene) { active_scene = std::unique_ptr<BaseScene>(scene); };

            Window& get_window() { return *window; };
            Renderer& get_renderer() { return *renderer; };
            Editor& get_editor() { return *editor; };
            ModelManager& get_model_manager() { return *model_loader; };
            TextureManager& get_texture_manager() { return *texture_loader; };
            ShaderManager& get_shader_manager() { return *shader_loader; };
            MaterialManager& get_material_manager() { return *material_loader; };
            PhysicsEngine& get_physics_engine() { return *physics_engine; };
            BaseScene& get_active_scene() { return *active_scene; };

        private:
            void on_window_close(WindowCloseEvent& e);
            void on_window_resize(WindowResizeEvent& e);

            std::unique_ptr<Window> window;
            std::unique_ptr<Renderer> renderer;
            std::unique_ptr<Editor> editor;
            std::unique_ptr<ModelManager> model_loader;
            std::unique_ptr<TextureManager> texture_loader;
            std::unique_ptr<MaterialManager> material_loader;
            std::unique_ptr<ShaderManager> shader_loader;
            std::unique_ptr<PhysicsEngine> physics_engine;
            std::unique_ptr<BaseScene> active_scene;

            b8 running;
    };

    // @TODO: idk if this is thread safe but i wont use singletons <:(
    Application& get_application();

    // Defined by the client
    Application* create_application();
};  // namespace mag
