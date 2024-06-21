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

    class BaseScene
    {
        public:
            BaseScene(const str name, ECS* ecs, Camera* camera, StandardRenderPass* render_pass)
                : name(name), ecs(ecs), camera(camera), render_pass(render_pass){};

            virtual ~BaseScene() = default;

            virtual void update(const f32 dt) { (void)dt; };
            virtual void on_event(Event& e)
            {
                EventDispatcher dispatcher(e);
                dispatcher.dispatch<ViewportResizeEvent>(BIND_FN(BaseScene::on_viewport_resize));
            };

            virtual void start_runtime() = 0;
            virtual void stop_runtime() = 0;

            virtual void add_model(const str& path) = 0;
            virtual void remove_model(const u32 id) = 0;

            virtual SceneState get_scene_state() const = 0;

            virtual ECS& get_ecs() { return *ecs; };
            const str& get_name() const { return name; };
            Camera& get_camera() { return *camera; };
            StandardRenderPass& get_render_pass() { return *render_pass; };

        protected:
            virtual void on_viewport_resize(ViewportResizeEvent& e)
            {
                const uvec2 size = {e.width, e.height};

                render_pass->on_resize(size);
                camera->set_aspect_ratio(size);
            };

            str name;
            std::unique_ptr<ECS> ecs;
            std::unique_ptr<Camera> camera;
            std::unique_ptr<StandardRenderPass> render_pass;
    };

    class Scene : public BaseScene
    {
        public:
            Scene();

            virtual void start_runtime() override;
            virtual void stop_runtime() override;

            virtual void update(const f32 dt) override;
            virtual void on_event(Event& e) override;

            virtual void add_model(const str& path) override;
            virtual void remove_model(const u32 id) override;

            virtual SceneState get_scene_state() const override { return current_state; };
            virtual ECS& get_ecs() override
            {
                if (current_state == SceneState::Editor)
                    return *ecs;

                else
                    return *runtime_ecs;
            };

        private:
            void update_runtime(const f32 dt);
            void update_editor(const f32 dt);

            std::unique_ptr<ECS> runtime_ecs;
            std::unique_ptr<EditorCameraController> camera_controller;
            std::unique_ptr<Cube> cube;

            SceneState current_state = SceneState::Editor;
            b8 state_swap_requested = false;
    };
};  // namespace mag
