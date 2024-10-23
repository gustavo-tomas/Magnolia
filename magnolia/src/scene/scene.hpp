#pragma once

#include <vector>

#include "core/event.hpp"

namespace mag
{
    class ECS;
    class Camera;
    struct Component;

    class Scene
    {
        public:
            Scene();
            virtual ~Scene();

            void on_start();
            void on_stop();

            void on_event(Event& e);
            void on_update(const f32 dt);

            // @TODO: this should extend to all components (and maybe be a bit more generic). This way if we ever need
            // to, we can do any extra necessary work after adding the entity/component to the ecs.
            void add_model(const str& path);
            void add_sprite(const str& path);

            void remove_entity(const u32 id);

            void set_name(const str& name);

            b8 is_running() const;

            const str& get_name() const;
            ECS& get_ecs();
            virtual Camera& get_camera();

        protected:
            // The user can override these if they want
            virtual void on_start_internal();
            virtual void on_stop_internal();
            virtual void on_event_internal(Event& e);
            virtual void on_update_internal(const f32 dt);
            virtual void on_resize(WindowResizeEvent& e);
            virtual void on_component_added(const u32 id, Component* component);

            str name;
            unique<ECS> ecs;

        private:
            std::vector<u32> entity_deletion_queue;
            b8 running = false;
    };
};  // namespace mag
