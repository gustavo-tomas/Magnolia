#pragma once

#include "camera/camera.hpp"
#include "camera/controller.hpp"
#include "core/event.hpp"
#include "ecs/ecs.hpp"
#include "renderer/model.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    enum class SceneState
    {
        Editor,
        Runtime
    };

    class Scene
    {
        public:
            Scene();
            ~Scene();

            void update(const f32 dt);
            void on_event(Event& e);

            void start_runtime();
            void stop_runtime();

            void add_model(const str& path);
            void remove_model(const u32 id);

            SceneState get_scene_state() const { return current_state; };

            const str& get_name() const { return name; };
            Camera& get_camera() { return *camera; };
            StandardRenderPass& get_render_pass() { return *render_pass; };

            ECS& get_ecs()
            {
                if (current_state == SceneState::Editor)
                    return *ecs;

                else
                    return *runtime_ecs;
            };

        private:
            void update_runtime(const f32 dt);
            void update_editor(const f32 dt);

            void on_viewport_resize(ViewportResizeEvent& e);

            str name;
            std::unique_ptr<ECS> ecs;
            std::unique_ptr<ECS> runtime_ecs;
            std::unique_ptr<Camera> camera;
            std::unique_ptr<EditorCameraController> camera_controller;
            std::unique_ptr<StandardRenderPass> render_pass;
            std::unique_ptr<Cube> cube;

            SceneState current_state = SceneState::Editor;
            b8 state_swap_requested = false;
    };
};  // namespace mag
