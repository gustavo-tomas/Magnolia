#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

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

            // Applies continuous force over time
            void apply_force(const u32 object_index, const math::vec3& force);

            // Applies an instantaneous change in momentum
            void apply_impulse(const u32 object_index, const math::vec3& impulse);

            // Applies continuous torque over time
            void apply_torque(const u32 object_index, const math::vec3& force);

            // Applies an instantaneous change in momentum
            void apply_torque_impulse(const u32 object_index, const math::vec3& force);

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
