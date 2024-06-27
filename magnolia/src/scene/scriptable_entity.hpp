#pragma once

#include "core/types.hpp"
#include "ecs/ecs.hpp"

namespace mag
{
    class ECS;
    class ScriptableEntity
    {
        public:
            virtual ~ScriptableEntity() = default;

        protected:
            virtual void on_create(){};
            virtual void on_destroy(){};
            virtual void on_update(const f32 dt) { (void)dt; };

            template <typename T>
            T* get_component()
            {
                return ecs->get_component<T>(entity_id);
            }

        private:
            friend class Scene;

            ECS* ecs;
            u32 entity_id;
    };
};  // namespace mag
