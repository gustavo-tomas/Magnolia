#pragma once

#include "core/types.hpp"
#include "ecs/ecs.hpp"

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
            virtual void on_signal_sent(const u32 target_id, const void* data);
            virtual void on_signal_received(const u32 sender_id, const void* data);

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
            std::tuple<Ts*...> get_external_entity_components(const u32 external_entity_id)
            {
                return ecs->get_components<Ts...>(external_entity_id);
            }

            template <typename... Ts>
            std::vector<u32> get_entities_with_components_of_type()
            {
                return ecs->get_entities_with_components_of_type<Ts...>();
            }

            u32 create_entity(const str& name = {}) const;

            template <typename T>
            void add_component_to_entity(const u32 entity_id, T* c)
            {
                ecs->add_component(entity_id, c);
            }

            IPhysicsWorld& get_physics_world() const;

        private:
            friend class Scene;

            Scene* scene = nullptr;
            IPhysicsWorld* physics_world = nullptr;
            ECS* ecs = nullptr;
            u32 entity_id = Invalid_ID;
    };
};  // namespace mag
