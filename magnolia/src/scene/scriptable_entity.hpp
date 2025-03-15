#pragma once

#include "core/types.hpp"
#include "ecs/ecs.hpp"

namespace mag
{
    struct Event;
    class PhysicsWorld;

    class ScriptableEntity
    {
        public:
            ScriptableEntity();
            virtual ~ScriptableEntity();

        protected:
            virtual void on_create();
            virtual void on_destroy();
            virtual void on_update(const f32 dt);
            virtual void on_event(const Event& e);

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
            std::tuple<Ts*...> get_external_entity_components(const u32 external_entity_id)
            {
                return ecs->get_components<Ts...>(external_entity_id);
            }

            template <typename... Ts>
            std::vector<u32> get_entities_with_components_of_type()
            {
                return ecs->get_entities_with_components_of_type<Ts...>();
            }

            PhysicsWorld& get_physics_world() const;

        private:
            friend class Scene;

            PhysicsWorld* physics_world = nullptr;
            ECS* ecs = nullptr;
            u32 entity_id;
    };
};  // namespace mag
