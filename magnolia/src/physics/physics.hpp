#pragma once

#include "core/types.hpp"

namespace mag::math
{
    struct LineList;
};

namespace mag
{
    class PhysicsDebugDraw;
    class Scene;

    struct PhysicsInternalData;
    struct TransformComponent;
    struct BoxColliderComponent;
    struct RigidBodyComponent;

    class PhysicsEngine
    {
        public:
            PhysicsEngine();
            ~PhysicsEngine();

            void on_simulation_start(Scene* scene);
            void on_simulation_end();

            void on_update(const f32 dt);

            const math::LineList& get_line_list() const;

        private:
            void add_rigid_body(const TransformComponent& transform, BoxColliderComponent& collider,
                                RigidBodyComponent& rigid_body);

            void remove_rigid_body(const u32 index);

            PhysicsInternalData* internal_data;
            unique<PhysicsDebugDraw> physics_debug_draw;
            Scene* scene;
    };
};  // namespace mag
