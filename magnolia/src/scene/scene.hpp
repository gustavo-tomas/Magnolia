#pragma once

#include <vector>

#include "core/event.hpp"

namespace mag
{
    class ECS;
    class Camera;
    class PhysicsWorld;
    struct Component;

    class Scene
    {
        public:
            Scene();
            virtual ~Scene();

            void on_start();
            void on_stop();

            void on_event(const Event& e);
            void on_update(const f32 dt);

            // @TODO: this should extend to all components (and maybe be a bit more generic). This way if we ever need
            // to, we can do any extra necessary work after adding the entity/component to the ecs.
            void add_model(const str& path);
            void add_sprite(const str& path);

            void remove_entity(const u32 id);

            void set_name(const str& name);

            b8 is_running() const;

            const str& get_name() const;
            const PhysicsWorld* get_physics_world() const;
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
            unique<PhysicsWorld> physics_world;

        private:
            void on_component_added(const u32 id, Component* component);
            void instantiate_scripts();
            void destroy_scripts();

            std::vector<u32> entity_deletion_queue;
            b8 running = false;
    };
};  // namespace mag
