#include "magnolia/physics/physics.hpp"

#include <unordered_map>

#include "BulletCollision/BroadphaseCollision/btDispatcher.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "LinearMath/btTransform.h"
#include "btBulletDynamicsCommon.h"
#include "magnolia/core/logger.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"
#include "physics/backend/bullet/conversions.hpp"

namespace mag
{
    // @TODO: finish debug draw
    class PhysicsDebugDraw : public btIDebugDraw
    {
        public:
            void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance,
                                  int lifeTime, const btVector3& color) override
            {
                (void)PointOnB;
                (void)normalOnB;
                (void)distance;
                (void)lifeTime;
                (void)color;
            }

            void reportErrorWarning(const c8* warning_string) override
            {
                LOG_ERROR("Physics Error: {0}", warning_string);
            }

            void draw3dText(const btVector3& location, const c8* text_string) override
            {
                (void)location;
                LOG_ERROR("3D text not supported: {0}", text_string);
            }

            void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
            {
                // We dont actually draw in this method, only keep a record of the lines.

                math::Line line = {};
                line.start = bt_to_mag(from);
                line.end = bt_to_mag(to);
                line.color = bt_to_mag(color);

                line_list.append(line);
            }

            // @TODO: finish debug mode
            void setDebugMode(int debugMode) override { (void)debugMode; }

            int getDebugMode() const override { return btIDebugDraw::DBG_DrawWireframe; }

            void reset_lines() { line_list.lines.clear(); }

            const math::LineList& get_line_list() const { return line_list; }

        private:
            math::LineList line_list;
    };

    class BulletPhysicsWorld : public IPhysicsWorld
    {
        public:
            BulletPhysicsWorld()
                : collision_configuration(new btDefaultCollisionConfiguration()),
                  dispatcher(new btCollisionDispatcher(collision_configuration)),
                  overlapping_pair_cache(new btDbvtBroadphase()),
                  solver(new btSequentialImpulseConstraintSolver()),
                  dynamics_world(
                      new btDiscreteDynamicsWorld(dispatcher, overlapping_pair_cache, solver, collision_configuration)),
                  physics_debug_draw(new PhysicsDebugDraw())
            {
                dynamics_world->setGravity(btVector3(0, -10, 0));

                dynamics_world->setDebugDrawer(physics_debug_draw.get());
            }

            ~BulletPhysicsWorld() override
            {
                // Cleanup in the reverse order of creation/initialization

                for (i32 i = dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--)
                {
                    btCollisionObject* bt_object = dynamics_world->getCollisionObjectArray().at(i);
                    btRigidBody* bt_rigid_body = dynamic_cast<btRigidBody*>(bt_object);

                    if (bt_rigid_body != nullptr && bt_rigid_body->getMotionState() != nullptr)
                    {
                        delete bt_rigid_body->getMotionState();
                    }

                    dynamics_world->removeCollisionObject(bt_object);

                    if (bt_object->getCollisionShape() != nullptr)
                    {
                        delete bt_object->getCollisionShape();
                    }

                    delete bt_object;
                }

                delete dynamics_world;
                delete solver;
                delete overlapping_pair_cache;
                delete dispatcher;
                delete collision_configuration;
            }

            static RigidBodyHandle create_handle()
            {
                static RigidBodyHandle handle_counter = 0;

                return handle_counter++;
            }

            // Base method to add generic body
            btRigidBody* add_rigid_body_base(const math::vec3& position, const math::quat& rotation, const f32 mass,
                                             btCollisionShape* shape) const
            {
                // Rigidbody is dynamic if and only if mass is non zero, otherwise static
                btVector3 local_inertia(0, 0, 0);
                if (mass > 0.0f)
                {
                    shape->calculateLocalInertia(mass, local_inertia);
                }

                btDefaultMotionState* motion_state = new btDefaultMotionState(mag_to_bt(position, rotation));

                btRigidBody::btRigidBodyConstructionInfo rb_info(mass, motion_state, shape, local_inertia);

                btRigidBody* bt_rigid_body = new btRigidBody(rb_info);

                dynamics_world->addRigidBody(bt_rigid_body);

                return bt_rigid_body;
            }

            RigidBodyHandle add_rigid_body(const math::vec3& position, const math::quat& rotation,
                                           const math::vec3& collider_dimensions, const f32 mass) override
            {
                btBoxShape* shape = new btBoxShape(mag_to_bt(collider_dimensions));
                btRigidBody* bt_rigid_body = add_rigid_body_base(position, rotation, mass, shape);

                const RigidBodyHandle handle = create_handle();

                rigid_bodies[handle] = bt_rigid_body;

                return handle;
            }

            RigidBodyHandle add_rigid_body(const math::vec3& position, const math::quat& rotation, const f32 radius,
                                           const f32 height, const f32 mass) override
            {
                btCapsuleShape* shape = new btCapsuleShape(radius, height);
                btRigidBody* bt_rigid_body = add_rigid_body_base(position, rotation, mass, shape);

                const RigidBodyHandle handle = create_handle();

                rigid_bodies[handle] = bt_rigid_body;

                return handle;
            }

            void remove_rigid_body(const RigidBodyHandle handle) override
            {
                btRigidBody* bt_rigid_body = rigid_bodies[handle];
                btCollisionObject* bt_object = static_cast<btCollisionObject*>(bt_rigid_body);

                if (bt_rigid_body != nullptr && bt_rigid_body->getMotionState() != nullptr)
                {
                    delete bt_rigid_body->getMotionState();
                }

                dynamics_world->removeCollisionObject(bt_object);

                if (bt_object->getCollisionShape() != nullptr)
                {
                    delete bt_object->getCollisionShape();
                }

                delete bt_object;

                rigid_bodies.erase(handle);
            }

            void reset_rigid_body(const RigidBodyHandle handle, const math::vec3& position, const math::quat& rotation,
                                  const math::vec3& collider_dimensions, const f32 mass) override
            {
                btRigidBody* bt_rigid_body = rigid_bodies[handle];

                // Update the collision shape
                btBoxShape* shape = new btBoxShape(mag_to_bt(collider_dimensions));

                btVector3 local_inertia(0, 0, 0);

                f32 new_mass = mass;
                if (mass < 0.0f)
                {
                    new_mass = bt_rigid_body->getMass();
                }

                // Calculate inertia if mass is non-zero
                if (new_mass > 0.0f)
                {
                    shape->calculateLocalInertia(new_mass, local_inertia);
                }

                if (bt_rigid_body->getCollisionShape() != nullptr)
                {
                    delete bt_rigid_body->getCollisionShape();
                }

                bt_rigid_body->setCollisionShape(shape);

                // Update collider properties

                const btTransform transform = mag_to_bt(position, rotation);

                bt_rigid_body->getMotionState()->setWorldTransform(transform);
                bt_rigid_body->setWorldTransform(transform);
                bt_rigid_body->setLinearVelocity(btVector3(0, 0, 0));
                bt_rigid_body->setAngularVelocity(btVector3(0, 0, 0));
                bt_rigid_body->setMassProps(new_mass, local_inertia);

                bt_rigid_body->clearForces();

                // Remove and add body to the world
                dynamics_world->removeRigidBody(bt_rigid_body);
                dynamics_world->addRigidBody(bt_rigid_body);

                bt_rigid_body->activate();
            }

            void on_update(const f32 dt) override
            {
                // @TODO: investigate the jittering that happens when maxSubSteps > 0.
                dynamics_world->stepSimulation(dt, 0);

                // 'Draw' the debug lines before sending to the renderer
                render_debug_lines();
            }

            void apply_force(const RigidBodyHandle handle, const math::vec3& force) override
            {
                btRigidBody* bt_rigid_body = rigid_bodies[handle];

                bt_rigid_body->applyCentralForce(mag_to_bt(force));
            }

            void apply_impulse(const RigidBodyHandle handle, const math::vec3& impulse) override
            {
                btRigidBody* bt_rigid_body = rigid_bodies[handle];

                // Don't forget to activate the body if it's sleeping
                bt_rigid_body->activate(true);

                bt_rigid_body->applyCentralImpulse(mag_to_bt(impulse));
            }

            void apply_torque(const RigidBodyHandle handle, const math::vec3& torque) override
            {
                btRigidBody* bt_rigid_body = rigid_bodies[handle];

                bt_rigid_body->applyTorque(mag_to_bt(torque));
            }

            void apply_torque_impulse(const RigidBodyHandle handle, const math::vec3& torque) override
            {
                btRigidBody* bt_rigid_body = rigid_bodies[handle];

                // Don't forget to activate the body if it's sleeping
                bt_rigid_body->activate(true);

                bt_rigid_body->applyTorqueImpulse(mag_to_bt(torque));
            }

            void set_linear_velocity(const RigidBodyHandle handle, const math::vec3& velocity) override
            {
                btRigidBody* bt_rigid_body = rigid_bodies[handle];

                // Don't forget to activate the body if it's sleeping
                bt_rigid_body->activate(true);

                bt_rigid_body->setLinearVelocity(mag_to_bt(velocity));
            }

            void set_angular_velocity(const RigidBodyHandle handle, const math::vec3& velocity) override
            {
                btRigidBody* bt_rigid_body = rigid_bodies[handle];

                // Don't forget to activate the body if it's sleeping
                bt_rigid_body->activate(true);

                bt_rigid_body->setAngularVelocity(mag_to_bt(velocity));
            }

            void set_angular_factor(const RigidBodyHandle handle, const math::vec3& axes) override
            {
                btRigidBody* bt_rigid_body = rigid_bodies[handle];

                bt_rigid_body->setAngularFactor(mag_to_bt(axes));
            }

            void set_activation_state(const RigidBodyHandle handle, const ActivationState activation_state) override
            {
                btRigidBody* bt_rigid_body = rigid_bodies[handle];

                bt_rigid_body->setActivationState(mag_to_bt(activation_state));
            }

            math::vec3 get_linear_velocity(const RigidBodyHandle handle) const override
            {
                btRigidBody* bt_rigid_body = rigid_bodies.at(handle);

                math::vec3 velocity = bt_to_mag(bt_rigid_body->getLinearVelocity());
                return velocity;
            }

            math::vec3 get_angular_velocity(const RigidBodyHandle handle) const override
            {
                btRigidBody* bt_rigid_body = rigid_bodies.at(handle);

                math::vec3 velocity = bt_to_mag(bt_rigid_body->getAngularVelocity());
                return velocity;
            }

            void get_collision_object_transform(const RigidBodyHandle handle, math::vec3& position,
                                                math::quat& rotation) const override
            {
                btRigidBody* bt_rigid_body = rigid_bodies.at(handle);

                btTransform bt_transform(btQuaternion(0, 0, 0, 0));

                if (bt_rigid_body != nullptr && bt_rigid_body->getMotionState() != nullptr)
                {
                    bt_rigid_body->getMotionState()->getWorldTransform(bt_transform);
                }

                else if (bt_rigid_body != nullptr)
                {
                    bt_transform = bt_rigid_body->getWorldTransform();
                }

                bt_to_mag(bt_transform, position, rotation);
            }

            const math::LineList& get_debug_line_list() const override { return physics_debug_draw->get_line_list(); }

        private:
            void render_debug_lines() override
            {
                physics_debug_draw->reset_lines();
                dynamics_world->debugDrawWorld();
            }

            btDefaultCollisionConfiguration* collision_configuration = nullptr;
            btCollisionDispatcher* dispatcher = nullptr;
            btBroadphaseInterface* overlapping_pair_cache = nullptr;
            btSequentialImpulseConstraintSolver* solver = nullptr;
            btDiscreteDynamicsWorld* dynamics_world = nullptr;
            unique<PhysicsDebugDraw> physics_debug_draw;
            std::unordered_map<RigidBodyHandle, btRigidBody*> rigid_bodies;
    };

    unique<IPhysicsWorld> create_physics_world() { return create_unique<BulletPhysicsWorld>(); }
};  // namespace mag
