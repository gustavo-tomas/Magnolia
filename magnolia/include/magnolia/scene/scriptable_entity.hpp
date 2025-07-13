#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/ecs/ecs.hpp"

namespace mag
{
    struct Event;
    class IPhysicsWorld;
    class Scene;

    class MAG_API ScriptableEntity
    {
        public:
            ScriptableEntity();
            virtual ~ScriptableEntity();

        protected:
            virtual void on_create();
            virtual void on_destroy();
            virtual void on_update(const f32 dt);
            virtual void on_event(const Event& e);
            virtual void on_signal_sent(const EntityID target_id, const void* data);
            virtual void on_signal_received(const EntityID sender_id, const void* data);

            void add_entity_to_deletion_queue();
            void set_active_scene(const str& scene_file_path);

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
            std::tuple<Ts*...> get_external_entity_components(const EntityID external_entity_id)
            {
                return ecs->get_components<Ts...>(external_entity_id);
            }

            template <typename... Ts>
            std::vector<EntityID> get_entities_with_components_of_type()
            {
                return ecs->query<Ts...>();
            }

            EntityID create_entity(const str& name = {}) const;

            template <typename T, typename... Args>
            void add_component_to_entity(const EntityID entity_id, Args&&... args)
            {
                ecs->add_component<T>(entity_id, std::forward<Args>(args)...);
            }

            IPhysicsWorld& get_physics_world() const;

        private:
            friend class Scene;

            Scene* scene = nullptr;
            IPhysicsWorld* physics_world = nullptr;
            ECS* ecs = nullptr;
            EntityID entity_id = Invalid_ID;
    };
};  // namespace mag
