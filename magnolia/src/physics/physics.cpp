#include "physics/physics.hpp"

#include "core/application.hpp"
#include "physics/type_conversions.hpp"

namespace mag
{
    PhysicsEngine::PhysicsEngine()
    {
        collision_configuration = new btDefaultCollisionConfiguration();

        dispatcher = new btCollisionDispatcher(collision_configuration);

        overlapping_pair_cache = new btDbvtBroadphase();

        solver = new btSequentialImpulseConstraintSolver();

        dynamics_world =
            new btDiscreteDynamicsWorld(dispatcher, overlapping_pair_cache, solver, collision_configuration);

        dynamics_world->setGravity(btVector3(0, -10, 0));

        physics_debug_draw = new PhysicsDebugDraw();

        dynamics_world->setDebugDrawer(physics_debug_draw);
    }

    PhysicsEngine::~PhysicsEngine()
    {
        // Cleanup in the reverse order of creation/initialization

        delete physics_debug_draw;

        for (i32 i = dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject* obj = dynamics_world->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);

            if (body && body->getMotionState())
            {
                delete body->getMotionState();
            }

            dynamics_world->removeCollisionObject(obj);

            if (obj->getCollisionShape())
            {
                delete obj->getCollisionShape();
            }

            delete obj;
        }

        delete dynamics_world;
        delete solver;
        delete overlapping_pair_cache;
        delete dispatcher;
        delete collision_configuration;

        for (auto collision_shape : collision_shapes) delete collision_shape;
        for (auto rigid_body : rigid_bodies) delete rigid_body;
    }

    BulletCollisionShape* PhysicsEngine::create_collision_shape(const vec3& box_half_extents)
    {
        auto* bt_shape = new btBoxShape(mag_vec_to_bt_vec(box_half_extents));
        BulletCollisionShape* collision_shape = new BulletCollisionShape(bt_shape);

        return collision_shape;
    }

    BulletRigidBody* PhysicsEngine::create_rigid_body(const BulletCollisionShape& shape,
                                                      const TransformComponent& transform, const f32 mass)
    {
        // Rigidbody is dynamic if and only if mass is non zero, otherwise static
        b8 is_dynamic = mass != 0.0f;

        btVector3 local_inertia(0, 0, 0);
        if (is_dynamic) shape.shape->calculateLocalInertia(mass, local_inertia);

        btDefaultMotionState* motion_state = new btDefaultMotionState(mag_transform_to_bt_transform(transform));

        btRigidBody::btRigidBodyConstructionInfo rb_info(mass, motion_state, shape.shape, local_inertia);

        btRigidBody* bt_rigid_body = new btRigidBody(rb_info);

        dynamics_world->addRigidBody(bt_rigid_body);

        BulletRigidBody* rigid_body = new BulletRigidBody(bt_rigid_body, mass);
        return rigid_body;
    }

    void PhysicsEngine::update(const f32 dt)
    {
        (void)dt;  // @TODO: figure out where dt goes

        auto& ecs = get_application().get_active_scene().get_ecs();
        auto objects = ecs.get_components_of_entities<TransformComponent, RigidBodyComponent>();

        dynamics_world->stepSimulation(1.0f / 60.0f, 10);

        for (i32 i = objects.size() - 1; i >= 0; i--)
        {
            auto [transform, rigid_body_c] = objects[i];
            auto body = rigid_body_c->rigid_body->rigid_body;

            btTransform trans;

            if (body && body->getMotionState())
            {
                body->getMotionState()->getWorldTransform(trans);
            }

            else
            {
                trans = body->getWorldTransform();
            }

            *transform = bt_transform_to_mag_transform(trans, transform->scale);
        }
    }

    void PhysicsEngine::render() { dynamics_world->debugDrawWorld(); }
};  // namespace mag
