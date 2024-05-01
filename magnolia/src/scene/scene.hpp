#pragma once

#include "camera/camera.hpp"
#include "camera/controller.hpp"
#include "core/event.hpp"
#include "renderer/model.hpp"
#include "renderer/render_pass.hpp"

namespace mag
{
    // @TODO: for now is basically just the models and a camera
    class Scene
    {
        public:
            void initialize();
            void shutdown();

            void update(const f32 dt);
            void on_event(Event& e);

            void add_model(const str& path);
            void set_camera(const Camera& camera) { this->camera = std::move(camera); }

            Camera& get_camera() { return camera; };
            std::vector<Model>& get_models() { return models; };
            StandardRenderPass& get_render_pass() { return render_pass; };

        private:
            Camera camera;
            EditorCameraController camera_controller;
            Cube cube;
            std::vector<Model> models;
            std::vector<str> models_queue;
            StandardRenderPass render_pass;
    };
};  // namespace mag
