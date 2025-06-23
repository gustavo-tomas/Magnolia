#pragma once

#include <vector>

#include "magnolia/core/event.hpp"

namespace mag
{
    class ECS;
    class Camera;
    class IPhysicsWorld;
    struct Component;
    struct ScriptComponent;

    class MAG_API Scene
    {
        public:
            Scene();
            virtual ~Scene();

            void on_start();
            void on_stop();

            void on_event(const Event& e);
            void on_update(const f32 dt);

            void remove_entity(const u32 id);

            void set_name(const str& name);
            void set_next_scene(const str& scene_file_path);

            b8 is_running() const;

            const str& get_name() const;
            const str& get_next_scene() const;
            const IPhysicsWorld* get_physics_world() const;
            ECS& get_ecs();
            virtual Camera& get_camera();

        protected:
            // The user can override these if they want
            virtual void on_start_internal();
            virtual void on_stop_internal();
            virtual void on_event_internal(const Event& e);
            virtual void on_update_internal(const f32 dt);
            virtual void on_component_added_internal(const u32 id, Component* component);
            virtual void on_resize(const WindowResizeEvent& e);

            str name;
            unique<ECS> ecs;
            unique<IPhysicsWorld> physics_world;

        private:
            void on_component_added(const u32 id, Component* component);
            void create_script(const u32 id);
            void destroy_script(ScriptComponent* script);

            std::vector<u32> entity_deletion_queue;
            str next_scene = "";
            b8 running = false;
    };
};  // namespace mag
