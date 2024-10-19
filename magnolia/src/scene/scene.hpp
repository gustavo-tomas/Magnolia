#pragma once

#include "camera/camera.hpp"
#include "core/assert.hpp"
#include "core/event.hpp"
#include "ecs/ecs.hpp"

namespace mag
{
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

            void set_name(const str& name) { this->name = name; };

            b8 is_running() const { return running; };

            const str& get_name() const { return name; };

            ECS& get_ecs() { return *ecs; };

            virtual Camera& get_camera()
            {
                // @TODO: for now we assume the active camera is the first entity with a camera component
                auto components = ecs->get_all_components_of_types<CameraComponent, TransformComponent>();
                for (auto [camera_c, transform] : components)
                {
                    return camera_c->camera;
                }

                ASSERT(false, "No runtime camera!");
                return std::get<0>(components[0])->camera;
            };

        protected:
            // The user can override these if they want
            virtual void on_start_internal(){};
            virtual void on_stop_internal(){};
            virtual void on_event_internal(Event& e) { (void)e; };
            virtual void on_update_internal(const f32 dt) { (void)dt; };
            virtual void on_resize(WindowResizeEvent& e);

            str name;
            unique<ECS> ecs;

        private:
            std::vector<u32> entity_deletion_queue;
            b8 running = false;
    };
};  // namespace mag
