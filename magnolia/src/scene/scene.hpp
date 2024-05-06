#pragma once

#include "camera/camera.hpp"
#include "camera/controller.hpp"
#include "core/event.hpp"
#include "ecs/ecs.hpp"
#include "renderer/model.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    // @TODO: for now is basically just the models and a camera
    // @TODO: create a base class for this
    class Scene
    {
        public:
            Scene();
            ~Scene();

            void update(const f32 dt);
            void on_event(Event& e);

            void add_model(const str& path);
            void set_camera(const Camera& camera) { this->camera = std::move(camera); }

            ECS& get_ecs() { return ecs; };
            Camera& get_camera() { return camera; };
            StandardRenderPass& get_render_pass() { return render_pass; };

        private:
            void on_viewport_resize(ViewportResizeEvent& e);

            ECS ecs;
            Camera camera;
            EditorCameraController camera_controller;
            StandardRenderPass render_pass;
            std::vector<std::unique_ptr<Cube>> cubes;
            std::vector<str> models_queue;
    };
};  // namespace mag
