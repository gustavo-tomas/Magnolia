#include "physics/physics.hpp"

#include "btBulletDynamicsCommon.h"
#include "core/logger.hpp"
#include "ecs/components.hpp"
#include "math/type_definitions.hpp"
#include "private/physics_type_conversions.hpp"
#include "scene/scene.hpp"

namespace mag
{
    struct PhysicsInternalData
    {
            btDefaultCollisionConfiguration* collision_configuration = nullptr;
            btCollisionDispatcher* dispatcher = nullptr;
            btBroadphaseInterface* overlapping_pair_cache = nullptr;
            btSequentialImpulseConstraintSolver* solver = nullptr;
            btDiscreteDynamicsWorld* dynamics_world = nullptr;
    };

    // @TODO: finish debug draw
    class PhysicsDebugDraw : public btIDebugDraw
    {
        public:
            void reset_lines();

            const LineList& get_line_list() const { return line_list; };

            virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
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

            virtual void setDebugMode(int debugMode) override { (void)debugMode; }  // @TODO: finish debug mode
            virtual int getDebugMode() const override { return btIDebugDraw::DBG_DrawWireframe; }

        private:
            LineList line_list;
    };

    PhysicsWorld::PhysicsWorld() : internal_data(new PhysicsInternalData()), physics_debug_draw(new PhysicsDebugDraw())
    {
        internal_data->collision_configuration = new btDefaultCollisionConfiguration();

        internal_data->dispatcher = new btCollisionDispatcher(internal_data->collision_configuration);

        internal_data->overlapping_pair_cache = new btDbvtBroadphase();

        internal_data->solver = new btSequentialImpulseConstraintSolver();

        internal_data->dynamics_world =
            new btDiscreteDynamicsWorld(internal_data->dispatcher, internal_data->overlapping_pair_cache,
                                        internal_data->solver, internal_data->collision_configuration);

        internal_data->dynamics_world->setGravity(btVector3(0, -10, 0));

        internal_data->dynamics_world->setDebugDrawer(physics_debug_draw.get());
    }

    PhysicsWorld::~PhysicsWorld()
    {
        // Cleanup in the reverse order of creation/initialization

        for (i32 i = internal_data->dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            remove_rigid_body(internal_data->dynamics_world->getCollisionObjectArray().at(i));
        }

        delete internal_data->dynamics_world;
        delete internal_data->solver;
        delete internal_data->overlapping_pair_cache;
        delete internal_data->dispatcher;
        delete internal_data->collision_configuration;
        delete internal_data;
    }

    void* PhysicsWorld::add_rigid_body(const math::vec3& position, const math::quat& rotation,
                                       const math::vec3& collider_dimensions, const f32 mass) const
    {
        btBoxShape* shape = new btBoxShape(mag_vec_to_bt_vec(collider_dimensions));

        // Rigidbody is dynamic if and only if mass is non zero, otherwise static
        btVector3 local_inertia(0, 0, 0);
        if (mass > 0.0f)
        {
            shape->calculateLocalInertia(mass, local_inertia);
        }

        btDefaultMotionState* motion_state =
            new btDefaultMotionState(mag_transform_to_bt_transform(position, rotation));

        btRigidBody::btRigidBodyConstructionInfo rb_info(mass, motion_state, shape, local_inertia);

        btRigidBody* bt_rigid_body = new btRigidBody(rb_info);

        internal_data->dynamics_world->addRigidBody(bt_rigid_body);

        return bt_rigid_body;
    }

    void PhysicsWorld::remove_rigid_body(void* collision_object)
    {
        btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
        btRigidBody* bt_rigid_body = static_cast<btRigidBody*>(bt_object);

        if (bt_rigid_body && bt_rigid_body->getMotionState())
        {
            delete bt_rigid_body->getMotionState();
        }

        internal_data->dynamics_world->removeCollisionObject(bt_object);

        if (bt_object->getCollisionShape())
        {
            delete bt_object->getCollisionShape();
        }

        delete bt_object;
    }

    void PhysicsWorld::reset_rigid_body(void* collision_object, const math::vec3& position, const math::vec3& rotation,
                                        const math::vec3& collider_dimensions, const f32 mass) const
    {
        btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
        btRigidBody* bt_rigid_body = static_cast<btRigidBody*>(bt_object);

        // Update the collision shape
        btBoxShape* shape = new btBoxShape(mag_vec_to_bt_vec(collider_dimensions));

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

        bt_rigid_body->getMotionState()->setWorldTransform(mag_transform_to_bt_transform(transform));
        bt_rigid_body->setWorldTransform(mag_transform_to_bt_transform(transform));
        bt_rigid_body->setLinearVelocity(btVector3(0, 0, 0));
        bt_rigid_body->setAngularVelocity(btVector3(0, 0, 0));
        bt_rigid_body->setMassProps(new_mass, local_inertia);

        bt_rigid_body->clearForces();

        // Remove and add body to the world
        internal_data->dynamics_world->removeRigidBody(bt_rigid_body);
        internal_data->dynamics_world->addRigidBody(bt_rigid_body);

        bt_rigid_body->activate();
    }

    void PhysicsWorld::on_update(const f32 dt)
    {
        // @TODO: investigate the jittering that happens when maxSubSteps > 0.
        internal_data->dynamics_world->stepSimulation(dt, 0);

        // 'Draw' the debug lines before sending to the renderer
        render_debug_lines();
    }

    void PhysicsWorld::apply_force(void* collision_object, const math::vec3& force)
    {
        btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
        btRigidBody* body = static_cast<btRigidBody*>(bt_object);

        body->applyCentralForce(mag_vec_to_bt_vec(force));
    }

    void PhysicsWorld::apply_impulse(void* collision_object, const math::vec3& impulse)
    {
        btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
        btRigidBody* body = static_cast<btRigidBody*>(bt_object);

        // Don't forget to activate the body if it's sleeping
        body->activate(true);

        body->applyCentralImpulse(mag_vec_to_bt_vec(impulse));
    }

    void PhysicsWorld::apply_torque(void* collision_object, const math::vec3& torque)
    {
        btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
        btRigidBody* body = static_cast<btRigidBody*>(bt_object);

        body->applyTorque(mag_vec_to_bt_vec(torque));
    }

    void PhysicsWorld::apply_torque_impulse(void* collision_object, const math::vec3& torque)
    {
        btCollisionObject* bt_object = static_cast<btCollisionObject*>(collision_object);
        btRigidBody* body = static_cast<btRigidBody*>(bt_object);

        // Don't forget to activate the body if it's sleeping
        body->activate(true);

        body->applyTorqueImpulse(mag_vec_to_bt_vec(torque));
    }

    void PhysicsWorld::render_debug_lines()
    {
        physics_debug_draw->reset_lines();
        internal_data->dynamics_world->debugDrawWorld();
    }

    void PhysicsWorld::get_collision_object_transform(void* collision_object, math::vec3& position,
                                                      math::vec3& rotation) const
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

        TransformComponent transform = bt_transform_to_mag_transform(bt_transform);

        position = transform.translation;
        rotation = transform.rotation;
    }

    const LineList& PhysicsWorld::get_line_list() const { return physics_debug_draw->get_line_list(); }

    void PhysicsDebugDraw::reset_lines()
    {
        line_list.starts.clear();
        line_list.ends.clear();
        line_list.colors.clear();
    }

    void PhysicsDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        // We dont actually draw in this method, only keep a record of the lines.
        line_list.starts.push_back(bt_vec_to_mag_vec(from));
        line_list.ends.push_back(bt_vec_to_mag_vec(to));
        line_list.colors.push_back(bt_vec_to_mag_vec(color));
    }
};  // namespace mag
