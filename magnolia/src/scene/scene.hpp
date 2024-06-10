#pragma once

#include "camera/camera.hpp"
#include "camera/controller.hpp"
#include "core/event.hpp"
#include "ecs/ecs.hpp"
#include "renderer/model.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    class BaseScene
    {
        public:
            BaseScene(ECS* ecs, Camera* camera, StandardRenderPass* render_pass)
                : ecs(ecs), camera(camera), render_pass(render_pass){};

            virtual ~BaseScene() = default;

            virtual void update(const f32 dt) { (void)dt; };
            virtual void on_event(Event& e)
            {
                EventDispatcher dispatcher(e);
                dispatcher.dispatch<ViewportResizeEvent>(BIND_FN(BaseScene::on_viewport_resize));
            };

            ECS& get_ecs() { return *ecs; };
            Camera& get_camera() { return *camera; };
            StandardRenderPass& get_render_pass() { return *render_pass; };

        protected:
            virtual void on_viewport_resize(ViewportResizeEvent& e)
            {
                const uvec2 size = {e.width, e.height};

                render_pass->on_resize(size);
                camera->set_aspect_ratio(size);
            };

            std::unique_ptr<ECS> ecs;
            std::unique_ptr<Camera> camera;
            std::unique_ptr<StandardRenderPass> render_pass;
    };

    class Scene : public BaseScene
    {
        public:
            Scene();

            virtual void update(const f32 dt) override;
            virtual void on_event(Event& e) override;

            void add_model(const str& path);
            void remove_model(const u32 id);

        private:
            std::unique_ptr<EditorCameraController> camera_controller;
            std::unique_ptr<Cube> cube;
            std::vector<str> models_queue;
    };
};  // namespace mag
