#pragma once

#include <any>
#include <magnolia/core/event.hpp>
#include <magnolia/ecs/ecs.hpp>
#include <magnolia/threads/job_system.hpp>
#include <vector>

namespace mag
{
    class ECS;
    class Camera;

    namespace physics
    {
        class IPhysicsWorld;
    };
};  // namespace mag

namespace game
{
    struct ScriptComponent;
    class Renderer;

    class Scene
    {
        public:
            Scene(Renderer* renderer);
            virtual ~Scene();

            void on_start();
            void on_stop();

            void on_event(const mag::Event& e);
            void on_update(const f32 dt);
            void on_render(const f32 dt);

            void remove_entity(const mag::EntityID id);

            void set_name(const str& name);
            void set_file_path(const str& file_path);
            void set_next_scene(const str& scene_file_path) { next_scene = scene_file_path; }

            b8 is_running() const;

            const str& get_name() const;
            const str& get_file_path() const;
            const str& get_next_scene() const;
            const mag::JobGroupHandle& get_job_group() const;
            mag::physics::IPhysicsWorld& get_physics_world();
            mag::ECS& get_ecs();
            virtual mag::Camera& get_camera();

        protected:
            // The user can override these if they want
            virtual void on_resize(const mag::WindowResizeEvent& e);

            str name = "Untitled";
            str file_path;
            mag::unique<mag::ECS> ecs;
            mag::unique<mag::physics::IPhysicsWorld> physics_world;

        private:
            void on_component_added(const mag::EntityID id, std::any& component);
            void create_script(const mag::EntityID id);
            void destroy_script(ScriptComponent* script);

            Renderer* renderer = nullptr;
            std::vector<mag::EntityID> entity_deletion_queue;
            str next_scene;
            mag::JobGroupHandle job_group;
            b8 running = false;
    };
};  // namespace game
