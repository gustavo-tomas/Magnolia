#pragma once

#include <any>
#include <magnolia/core/event.hpp>
#include <magnolia/ecs/ecs.hpp>

namespace mag
{
    class ECS;
    class Camera;
};  // namespace mag

namespace game
{
    struct ScriptComponent;

    class Scene
    {
        public:
            Scene();
            virtual ~Scene();

            void on_start();
            void on_stop();

            void on_event(const mag::Event& e);
            void on_update(const f32 dt);

            b8 is_running() const;

            mag::ECS& get_ecs();
            virtual mag::Camera& get_camera();

        protected:
            // The user can override these if they want
            virtual void on_resize(const mag::WindowResizeEvent& e);

            mag::unique<mag::ECS> ecs;

        private:
            void on_component_added(const mag::EntityID id, std::any component);

            b8 running = false;
    };
};  // namespace game
