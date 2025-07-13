#pragma once

#include <any>
#include <vector>

#include "magnolia/core/event.hpp"
#include "magnolia/ecs/ecs.hpp"

namespace mag
{
    class ECS;
    class Camera;
    class IPhysicsWorld;
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

            void remove_entity(const EntityID id);

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
            virtual void on_resize(const WindowResizeEvent& e);

            str name;
            unique<ECS> ecs;
            unique<IPhysicsWorld> physics_world;

        private:
            void on_component_added(const EntityID id, std::any component);
            void create_script(const EntityID id);
            void destroy_script(ScriptComponent* script);

            std::vector<EntityID> entity_deletion_queue;
            str next_scene = "";
            b8 running = false;
    };
};  // namespace mag
