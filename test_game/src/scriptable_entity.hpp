#pragma once

#include <magnolia/core/types.hpp>
#include <magnolia/ecs/ecs.hpp>

#include "components.hpp"
#include "scene.hpp"

namespace mag
{
    struct Event;
    class IPhysicsWorld;
    class Camera;
};  // namespace mag

namespace game
{
    // @TODO: scripts are quite limited and don't work very well with a data oriented ECS. I think the best way to
    // handle them right now is to use them to extend functionalities of the engine (like the debug script) and work
    // mostly independent of the user application.
    class ScriptableEntity
    {
        public:
            ScriptableEntity() = default;
            virtual ~ScriptableEntity() = default;

        protected:
            virtual void on_create() {}
            virtual void on_destroy() {}
            virtual void on_update(const f32 dt) { (void)dt; }
            virtual void on_render(const f32 dt) { (void)dt; }
            virtual void on_event(const mag::Event& e) { (void)e; }

            virtual void on_signal_sent(const mag::EntityID target_id, const void* data)
            {
                ScriptComponent* script = ecs->get_component<ScriptComponent>(target_id);

                if ((script == nullptr) || (script->entity == nullptr))
                {
                    return;
                }

                // Send signal to target entity
                script->entity->on_signal_received(entity_id, data);
            }

            virtual void on_signal_received(const mag::EntityID sender_id, const void* data)
            {
                (void)sender_id;
                (void)data;
            }

            void add_entity_to_deletion_queue() { scene->remove_entity(entity_id); }

            void set_active_scene(const str& scene_file_path) { scene->set_next_scene(scene_file_path); }

            mag::EntityID create_entity(const str& name) const
            {
                const mag::EntityID entity_id = ecs->create_entity();

                if (!name.empty())
                {
                    ecs->add_component<NameComponent>(entity_id, name);
                }

                return entity_id;
            }

            mag::physics::IPhysicsWorld& get_physics_world() const { return *physics_world; }

            mag::Camera& get_camera() { return scene->get_camera(); }

            template <typename T>
            T* get_component()
            {
                return ecs->get_component<T>(entity_id);
            }

            template <typename... Ts>
            std::tuple<Ts*...> get_components()
            {
                return ecs->get_components<Ts...>(entity_id);
            }

            template <typename... Ts>
            std::vector<std::tuple<Ts*...>> get_all_components_of_types()
            {
                return ecs->get_all_components_of_types<Ts...>();
            }

            template <typename... Ts>
            std::tuple<Ts*...> get_external_entity_components(const mag::EntityID external_entity_id)
            {
                return ecs->get_components<Ts...>(external_entity_id);
            }

            template <typename... Ts>
            std::vector<mag::EntityID> get_entities_with_components_of_type()
            {
                return ecs->query<Ts...>();
            }

            template <typename T, typename... Args>
            void add_component_to_entity(const mag::EntityID entity_id, Args&&... args)
            {
                ecs->add_component<T>(entity_id, std::forward<Args>(args)...);
            }

        private:
            friend class Scene;

            Scene* scene = nullptr;
            mag::physics::IPhysicsWorld* physics_world = nullptr;
            mag::ECS* ecs = nullptr;
            mag::EntityID entity_id = mag::Invalid_ID;
    };
};  // namespace game
