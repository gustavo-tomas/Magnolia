#pragma once

#include "core/types.hpp"
#include "ecs/ecs.hpp"

namespace mag
{
    struct Event;

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

        private:
            friend class Scene;

            ECS* ecs = nullptr;
            u32 entity_id;
    };
};  // namespace mag
