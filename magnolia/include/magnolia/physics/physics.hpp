#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag::math
{
    struct LineList;
};

namespace mag
{
    enum class ActivationState
    {
        DisableDeactivation = 1
    };

    class MAG_API IPhysicsWorld
    {
        public:
            virtual ~IPhysicsWorld() = default;

            virtual void on_update(const f32 dt) = 0;

            // Add a rigid body with a box collider
            virtual void* add_rigid_body(const math::vec3& position, const math::quat& rotation,
                                         const math::vec3& collider_dimensions, const f32 mass) const = 0;

            // Add a rigid body with a capsule collider
            virtual void* add_rigid_body(const math::vec3& position, const math::quat& rotation, const f32 radius,
                                         const f32 height, const f32 mass) const = 0;

            virtual void remove_rigid_body(void* collision_object) = 0;

            virtual void reset_rigid_body(void* collision_object, const math::vec3& position,
                                          const math::vec3& rotation, const math::vec3& collider_dimensions,
                                          const f32 mass = -1.0f) const = 0;

            // Applies continuous force over time
            virtual void apply_force(void* collision_object, const math::vec3& force) = 0;

            // Applies an instantaneous change in momentum
            virtual void apply_impulse(void* collision_object, const math::vec3& impulse) = 0;

            // Applies continuous torque over time
            virtual void apply_torque(void* collision_object, const math::vec3& force) = 0;

            // Applies an instantaneous change in torque
            virtual void apply_torque_impulse(void* collision_object, const math::vec3& force) = 0;

            // Set body linear velocity
            virtual void set_linear_velocity(void* collision_object, const math::vec3& velocity) = 0;

            // Set body angular velocity
            virtual void set_angular_velocity(void* collision_object, const math::vec3& velocity) = 0;

            // Set body angular factor
            virtual void set_angular_factor(void* collision_object, const math::vec3& axes) = 0;

            // Set body activation state
            virtual void set_activation_state(void* collision_object, const ActivationState activation_state) = 0;

            // Get linear velocity of a body
            virtual math::vec3 get_linear_velocity(void* collision_object) const = 0;

            // Get angular velocity of a body
            virtual math::vec3 get_angular_velocity(void* collision_object) const = 0;

            // Get current transform of a collision object
            virtual void get_collision_object_transform(void* collision_object, math::vec3& position,
                                                        math::vec3& rotation) const = 0;

            // Debug lines for visualization
            virtual const math::LineList& get_debug_line_list() const = 0;

        private:
            virtual void render_debug_lines() = 0;
    };

    MAG_API unique<IPhysicsWorld> create_physics_world();
};  // namespace mag
