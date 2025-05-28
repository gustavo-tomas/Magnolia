#include "physics/physics.hpp"

#include "btBulletDynamicsCommon.h"
#include "core/logger.hpp"
#include "core/types.hpp"
#include "ecs/components.hpp"
#include "math/types.hpp"
#include "physics/backend/bullet/conversions.hpp"
#include "scene/scene.hpp"

namespace mag
{
    // @TODO: finish debug draw
    class PhysicsDebugDraw : public btIDebugDraw
    {
        public:
            virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance,
                                          int lifeTime, const btVector3& color) override
            {
                (void)PointOnB;
                (void)normalOnB;
                (void)distance;
                (void)lifeTime;
                (void)color;

                return;
            }

            virtual void reportErrorWarning(const c8* warning_string) override
            {
                LOG_ERROR("Physics Error: {0}", warning_string);
            }

            virtual void draw3dText(const btVector3& location, const c8* text_string) override
            {
                (void)location;
                LOG_ERROR("3D text not supported: {0}", text_string);
            }

            virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
            {
                // We dont actually draw in this method, only keep a record of the lines.
                line_list.starts.push_back(bt_to_mag(from));
                line_list.ends.push_back(bt_to_mag(to));
                line_list.colors.push_back(bt_to_mag(color));
            }

            // @TODO: finish debug mode
            virtual void setDebugMode(int debugMode) override { (void)debugMode; }

            virtual int getDebugMode() const override { return btIDebugDraw::DBG_DrawWireframe; }

            void reset_lines()
            {
                line_list.starts.clear();
                line_list.ends.clear();
                line_list.colors.clear();
            }

            const LineList& get_line_list() const { return line_list; }

        private:
            LineList line_list;
    };

    class BulletPhysicsWorld : public IPhysicsWorld
    {
        public:
            BulletPhysicsWorld() : physics_debug_draw(new PhysicsDebugDraw())
            {
                collision_configuration = new btDefaultCollisionConfiguration();

                dispatcher = new btCollisionDispatcher(collision_configuration);

                overlapping_pair_cache = new btDbvtBroadphase();

                solver = new btSequentialImpulseConstraintSolver();

                dynamics_world =
                    new btDiscreteDynamicsWorld(dispatcher, overlapping_pair_cache, solver, collision_configuration);

                dynamics_world->setGravity(btVector3(0, -10, 0));

                dynamics_world->setDebugDrawer(physics_debug_draw.get());
            }

            ~BulletPhysicsWorld()
            {
                // Cleanup in the reverse order of creation/initialization

                for (i32 i = dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--)
                {
                    remove_rigid_body(dynamics_world->getCollisionObjectArray().at(i));
                }

                delete dynamics_world;
                delete solver;
                delete overlapping_pair_cache;
                delete dispatcher;
                delete collision_configuration;
            }

            virtual void* add_rigid_body(const math::vec3& position, const math::quat& rotation,
                                         const math::vec3& collider_dimensions, const f32 mass) const override
            {
                btBoxShape* shape = new btBoxShape(mag_to_bt(collider_dimensions));

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

            virtual void remove_rigid_body(void* collision_object) override
            {
                btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
                btRigidBody* bt_rigid_body = static_cast<btRigidBody*>(bt_object);

                if (bt_rigid_body && bt_rigid_body->getMotionState())
                {
                    delete bt_rigid_body->getMotionState();
                }

                dynamics_world->removeCollisionObject(bt_object);

                if (bt_object->getCollisionShape())
                {
                    delete bt_object->getCollisionShape();
                }

                delete bt_object;
            }

            virtual void reset_rigid_body(void* collision_object, const math::vec3& position,
                                          const math::vec3& rotation, const math::vec3& collider_dimensions,
                                          const f32 mass) const override
            {
                btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
                btRigidBody* bt_rigid_body = static_cast<btRigidBody*>(bt_object);

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

                if (bt_rigid_body->getCollisionShape())
                {
                    delete bt_rigid_body->getCollisionShape();
                }

                bt_rigid_body->setCollisionShape(shape);

                // Update collider properties
                TransformComponent transform = TransformComponent(position, rotation);

                bt_rigid_body->getMotionState()->setWorldTransform(mag_to_bt(transform));
                bt_rigid_body->setWorldTransform(mag_to_bt(transform));
                bt_rigid_body->setLinearVelocity(btVector3(0, 0, 0));
                bt_rigid_body->setAngularVelocity(btVector3(0, 0, 0));
                bt_rigid_body->setMassProps(new_mass, local_inertia);

                bt_rigid_body->clearForces();

                // Remove and add body to the world
                dynamics_world->removeRigidBody(bt_rigid_body);
                dynamics_world->addRigidBody(bt_rigid_body);

                bt_rigid_body->activate();
            }

            virtual void on_update(const f32 dt) override
            {
                // @TODO: investigate the jittering that happens when maxSubSteps > 0.
                dynamics_world->stepSimulation(dt, 0);

                // 'Draw' the debug lines before sending to the renderer
                render_debug_lines();
            }

            virtual void apply_force(void* collision_object, const math::vec3& force) override
            {
                btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
                btRigidBody* body = static_cast<btRigidBody*>(bt_object);

                body->applyCentralForce(mag_to_bt(force));
            }

            virtual void apply_impulse(void* collision_object, const math::vec3& impulse) override
            {
                btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
                btRigidBody* body = static_cast<btRigidBody*>(bt_object);

                // Don't forget to activate the body if it's sleeping
                body->activate(true);

                body->applyCentralImpulse(mag_to_bt(impulse));
            }

            virtual void apply_torque(void* collision_object, const math::vec3& torque) override
            {
                btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
                btRigidBody* body = static_cast<btRigidBody*>(bt_object);

                body->applyTorque(mag_to_bt(torque));
            }

            virtual void apply_torque_impulse(void* collision_object, const math::vec3& torque) override
            {
                btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
                btRigidBody* body = static_cast<btRigidBody*>(bt_object);

                // Don't forget to activate the body if it's sleeping
                body->activate(true);

                body->applyTorqueImpulse(mag_to_bt(torque));
            }

            virtual void get_collision_object_transform(void* collision_object, math::vec3& position,
                                                        math::vec3& rotation) const override
            {
                btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
                btRigidBody* bt_body = static_cast<btRigidBody*>(bt_object);

                btTransform bt_transform(btQuaternion(0, 0, 0, 0));

                if (bt_body && bt_body->getMotionState())
                {
                    bt_body->getMotionState()->getWorldTransform(bt_transform);
                }

                else if (bt_body)
                {
                    bt_transform = bt_body->getWorldTransform();
                }

                TransformComponent transform = bt_to_mag(bt_transform);

                position = transform.translation;
                rotation = transform.rotation;
            }

            virtual const LineList& get_debug_line_list() const override { return physics_debug_draw->get_line_list(); }

        private:
            virtual void render_debug_lines() override
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
    };

    unique<IPhysicsWorld> create_physics_world() { return create_unique<BulletPhysicsWorld>(); }
};  // namespace mag
