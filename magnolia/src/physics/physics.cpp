#include "physics/physics.hpp"

#include "core/application.hpp"
#include "physics/type_conversions.hpp"

namespace mag
{
    PhysicsEngine::PhysicsEngine() : physics_debug_draw(new PhysicsDebugDraw()) {}

    PhysicsEngine::~PhysicsEngine() { on_simulation_end(); }

    void PhysicsEngine::on_simulation_start()
    {
        if (dynamics_world) on_simulation_end();

        collision_configuration = new btDefaultCollisionConfiguration();

        dispatcher = new btCollisionDispatcher(collision_configuration);

        overlapping_pair_cache = new btDbvtBroadphase();

        solver = new btSequentialImpulseConstraintSolver();

        dynamics_world =
            new btDiscreteDynamicsWorld(dispatcher, overlapping_pair_cache, solver, collision_configuration);

        dynamics_world->setGravity(btVector3(0, -10, 0));

        dynamics_world->setDebugDrawer(physics_debug_draw.get());

        auto& ecs = get_application().get_active_scene().get_ecs();
        auto objects = ecs.get_components_of_entities<TransformComponent, BoxColliderComponent, RigidBodyComponent>();
        for (auto& [transform, collider, rigid_body] : objects)
        {
            add_rigid_body(*transform, *collider, *rigid_body);
        }
    }

    void PhysicsEngine::on_simulation_end()
    {
        // Cleanup in the reverse order of creation/initialization

        if (!dynamics_world) return;

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
    }

    void PhysicsEngine::add_rigid_body(const TransformComponent& transform, BoxColliderComponent& collider,
                                       RigidBodyComponent& rigid_body)
    {
        auto* shape = new btBoxShape(mag_vec_to_bt_vec(collider.dimensions));

        // Rigidbody is dynamic if and only if mass is non zero, otherwise static
        btVector3 local_inertia(0, 0, 0);
        if (rigid_body.is_dynamic()) shape->calculateLocalInertia(rigid_body.mass, local_inertia);

        btDefaultMotionState* motion_state = new btDefaultMotionState(mag_transform_to_bt_transform(transform));

        btRigidBody::btRigidBodyConstructionInfo rb_info(rigid_body.mass, motion_state, shape, local_inertia);

        btRigidBody* bt_rigid_body = new btRigidBody(rb_info);

        dynamics_world->addRigidBody(bt_rigid_body);

        collider.internal = shape;
        rigid_body.internal = bt_rigid_body;
    }

    void PhysicsEngine::update(const f32 dt)
    {
        (void)dt;  // @TODO: figure out where dt goes

        auto& app = get_application();
        auto& scene = app.get_active_scene();
        auto& ecs = scene.get_ecs();
        auto objects = ecs.get_components_of_entities<TransformComponent, RigidBodyComponent>();

        physics_debug_draw->reset_lines();

        if (!dynamics_world) return;

        if (scene.get_scene_state() == SceneState::Runtime)
        {
            dynamics_world->stepSimulation(1.0f / 60.0f, 10);

            for (i32 i = objects.size() - 1; i >= 0; i--)
            {
                auto [transform, rigid_body_c] = objects[i];
                auto* body = static_cast<btRigidBody*>(rigid_body_c->internal);

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
        dynamics_world->debugDrawWorld();
    }

    const DebugLineList& PhysicsEngine::get_line_list() const { return physics_debug_draw->get_line_list(); };

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
