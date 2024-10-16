#pragma once

#include "camera/camera.hpp"
#include "camera/controller.hpp"
#include "scene/scene.hpp"

namespace mag
{
    class EditorScene : public Scene
    {
        public:
            EditorScene();

            // Send the editor camera as the active camera in editor mode
            virtual Camera& get_camera() override
            {
                if (is_running())
                {
                    auto components = ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
                    for (auto [camera_c, transform] : components)
                    {
                        return camera_c->camera;
                    }

                    ASSERT(false, "No runtime camera!");
                    return std::get<0>(components[0])->camera;
                }

                else
                {
                    return *camera;
                }
            };

            void on_viewport_resize(const uvec2& new_viewport_size);

        protected:
            virtual void on_start_internal() override;
            virtual void on_stop_internal() override;
            virtual void on_event_internal(Event& e) override;
            virtual void on_update_internal(const f32 dt) override;
            virtual void on_resize(WindowResizeEvent& e) override;

        private:
            unique<ECS> temporary_ecs;
            unique<Camera> camera;
            unique<EditorCameraController> camera_controller;
            uvec2 current_viewport_size;
    };
};  // namespace mag
