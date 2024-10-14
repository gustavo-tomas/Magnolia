#pragma once

#include "ecs/components.hpp"
#include "scene/scene.hpp"

namespace mag
{
    class PhysicsDebugDraw;
    struct PhysicsInternalData;

    class PhysicsEngine
    {
        public:
            PhysicsEngine();
            ~PhysicsEngine();

            void on_simulation_start(Scene* scene);
            void on_simulation_end();

            void on_update(const f32 dt);

            const LineList& get_line_list() const;

        private:
            void add_rigid_body(const TransformComponent& transform, BoxColliderComponent& collider,
                                RigidBodyComponent& rigid_body);

            void remove_rigid_body(const u32 index);

            PhysicsInternalData* internal_data;
            unique<PhysicsDebugDraw> physics_debug_draw;
            Scene* scene;
    };
};  // namespace mag
