#pragma once

#include "math/types.hpp"
#include "math/vec.hpp"
#include "scene/scene.hpp"

namespace mag
{
    class Camera;
};

namespace sprout
{
    using namespace mag;

    class EditorCameraController;

    class EditorScene : public Scene
    {
        public:
            EditorScene();
            ~EditorScene();

            // Send the editor camera as the active camera in editor mode
            virtual Camera& get_camera() override;

            void on_viewport_resize(const math::uvec2& new_viewport_size);

        protected:
            virtual void on_start_internal() override;
            virtual void on_stop_internal() override;
            virtual void on_event_internal(Event& e) override;
            virtual void on_update_internal(const f32 dt) override;
            virtual void on_resize(WindowResizeEvent& e) override;
            virtual void on_component_added(const u32 id, Component* component) override;

        private:
            unique<ECS> temporary_ecs;
            unique<Camera> camera;
            unique<EditorCameraController> camera_controller;
            math::uvec2 current_viewport_size;
    };
};  // namespace sprout
