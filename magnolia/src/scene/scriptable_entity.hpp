#pragma once

#include <functional>

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

    class Script
    {
        private:
            friend class Scene;
            friend class ScriptingEngine;  // @TODO: is this a good idea?

            std::function<void(Script&)> on_create;
            std::function<void(Script&)> on_destroy;
            std::function<void(Script&, const f32)> on_update;

            template <typename T>
            T* get_component()
            {
                return ecs->get_component<T>(entity_id);
            }

            ECS* ecs = nullptr;
            u32 entity_id;
    };
};  // namespace mag
