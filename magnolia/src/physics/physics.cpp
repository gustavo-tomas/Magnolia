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

        auto& app = get_application();
        auto& scene = app.get_active_scene();
        auto& ecs = scene.get_ecs();
        auto objects = ecs.get_components_of_entities<TransformComponent, RigidBodyComponent>();

        if (scene.get_scene_state() == SceneState::Runtime)
        {
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

        // 'Draw' the debug lines before sending to the renderer
        physics_debug_draw->reset_lines();
        dynamics_world->debugDrawWorld();
        physics_debug_draw->create_lines();
    }

    const std::unique_ptr<Line>& PhysicsEngine::get_line_list() const { return physics_debug_draw->get_line_list(); };

    void PhysicsDebugDraw::reset_lines()
    {
        starts.clear();
        ends.clear();
        colors.clear();
        line_list.reset();
    }

    void PhysicsDebugDraw::create_lines() { line_list = std::make_unique<Line>("DebugLines", starts, ends, colors); }

    void PhysicsDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
    {
        // We dont actually draw in this method, only keep a record of the lines.
        starts.push_back(bt_vec_to_mag_vec(from));
        ends.push_back(bt_vec_to_mag_vec(to));
        colors.push_back(bt_vec_to_mag_vec(color));
    }
};  // namespace mag
