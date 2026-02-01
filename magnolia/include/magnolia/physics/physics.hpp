#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    using RigidBodyHandle = u32;

    enum class ActivationState : u8
    {
        Activate,
        Deactivate
    };

    namespace physics
    {
        b8 initialize();

        void shutdown();

        class MAG_API IPhysicsWorld
        {
            public:
                virtual ~IPhysicsWorld() = default;

                virtual void on_update(const f32 dt) = 0;

                // Add a rigid body with a box collider
                virtual RigidBodyHandle add_rigid_body(const math::vec3& position, const math::quat& rotation,
                                                       const math::vec3& collider_dimensions, const f32 mass) = 0;

                // Add a rigid body with a capsule collider
                virtual RigidBodyHandle add_rigid_body(const math::vec3& position, const math::quat& rotation,
                                                       const f32 radius, const f32 height, const f32 mass) = 0;

                virtual void remove_rigid_body(const RigidBodyHandle handle) = 0;

                // Applies continuous force over time
                virtual void apply_force(const RigidBodyHandle handle, const math::vec3& force) = 0;

                // Applies an instantaneous change in momentum
                virtual void apply_impulse(const RigidBodyHandle handle, const math::vec3& impulse) = 0;

                // Applies continuous torque over time
                virtual void apply_torque(const RigidBodyHandle handle, const math::vec3& force) = 0;

                // Applies an instantaneous change in torque
                virtual void apply_torque_impulse(const RigidBodyHandle handle, const math::vec3& force) = 0;

                // Set body linear velocity
                virtual void set_linear_velocity(const RigidBodyHandle handle, const math::vec3& velocity) = 0;

                // Set body angular velocity
                virtual void set_angular_velocity(const RigidBodyHandle handle, const math::vec3& velocity) = 0;

                // Set body angular factor
                virtual void set_angular_factor(const RigidBodyHandle handle, const math::vec3& axes) = 0;

                // Set body activation state
                virtual void set_activation_state(const RigidBodyHandle handle,
                                                  const ActivationState activation_state) = 0;

                // Get linear velocity of a body
                virtual math::vec3 get_linear_velocity(const RigidBodyHandle handle) const = 0;

                // Get angular velocity of a body
                virtual math::vec3 get_angular_velocity(const RigidBodyHandle handle) const = 0;

                // Get current transform of a collision object
                virtual void get_collision_object_transform(const RigidBodyHandle handle, math::vec3& position,
                                                            math::quat& rotation) const = 0;

                // Debug lines for visualization
                virtual const math::LineList& get_debug_line_list() const = 0;
        };

        MAG_API unique<IPhysicsWorld> create_physics_world();
    };  // namespace physics
};  // namespace mag
