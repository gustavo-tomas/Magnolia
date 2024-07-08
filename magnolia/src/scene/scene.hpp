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
            void remove_entity(const u32 id);

            SceneState get_scene_state() const { return current_state; };

            const str& get_name() const { return name; };

            StandardRenderPass& get_render_pass() { return *render_pass; };

            Camera& get_camera()
            {
                if (current_state == SceneState::Editor) return *camera;

                // @TODO: for now we assume the active camera is the first entity with a camera component
                else
                {
                    auto components = runtime_ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
                    for (auto [camera_c, transform] : components)
                    {
                        return camera_c->camera;
                    }
                }

                ASSERT(false, "No runtime camera!");
            };

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
            std::vector<u32> editor_deletion_queue;
    };
};  // namespace mag
