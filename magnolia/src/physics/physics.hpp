#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag::math
{
    struct LineList;
};

namespace mag
{
    class PhysicsWorld
    {
        public:
            PhysicsWorld();
            ~PhysicsWorld();

            void on_update(const f32 dt);

            void* add_rigid_body(const math::vec3& position, const math::quat& rotation,
                                 const math::vec3& collider_dimensions, const f32 mass) const;

            void remove_rigid_body(void* collision_object);

            void reset_rigid_body(void* collision_object, const math::vec3& position, const math::vec3& rotation,
                                  const math::vec3& collider_dimensions, const f32 mass = -1.0f) const;

            // Applies continuous force over time
            void apply_force(void* collision_object, const math::vec3& force);

            // Applies an instantaneous change in momentum
            void apply_impulse(void* collision_object, const math::vec3& impulse);

            // Applies continuous torque over time
            void apply_torque(void* collision_object, const math::vec3& force);

            // Applies an instantaneous change in torque
            void apply_torque_impulse(void* collision_object, const math::vec3& force);

            // Get current transform of a collision object
            void get_collision_object_transform(void* collision_object, math::vec3& position,
                                                math::vec3& rotation) const;

            const math::LineList& get_line_list() const;

        private:
            void render_debug_lines();

            struct PhysicsInternalData* internal_data = nullptr;
            unique<class PhysicsDebugDraw> physics_debug_draw;
    };
};  // namespace mag
