#include "physics/physics.hpp"

#include "btBulletDynamicsCommon.h"
#include "core/logger.hpp"
#include "core/math.hpp"
#include "ecs/ecs.hpp"
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

    PhysicsEngine::PhysicsEngine()
        : internal_data(new PhysicsInternalData()), physics_debug_draw(new PhysicsDebugDraw())
    {
    }

    PhysicsEngine::~PhysicsEngine()
    {
        on_simulation_end();
        delete internal_data;
    }

    void PhysicsEngine::on_simulation_start(Scene* scene)
    {
        if (internal_data->dynamics_world) on_simulation_end();

        this->scene = scene;

        internal_data->collision_configuration = new btDefaultCollisionConfiguration();

        internal_data->dispatcher = new btCollisionDispatcher(internal_data->collision_configuration);

        internal_data->overlapping_pair_cache = new btDbvtBroadphase();

        internal_data->solver = new btSequentialImpulseConstraintSolver();

        internal_data->dynamics_world =
            new btDiscreteDynamicsWorld(internal_data->dispatcher, internal_data->overlapping_pair_cache,
                                        internal_data->solver, internal_data->collision_configuration);

        internal_data->dynamics_world->setGravity(btVector3(0, -10, 0));

        internal_data->dynamics_world->setDebugDrawer(physics_debug_draw.get());

        auto& ecs = scene->get_ecs();
        auto objects = ecs.get_all_components_of_types<TransformComponent, BoxColliderComponent, RigidBodyComponent>();
        for (auto [transform, collider, rigid_body] : objects)
        {
            add_rigid_body(*transform, *collider, *rigid_body);
        }
    }

    void PhysicsEngine::on_simulation_end()
    {
        // Cleanup in the reverse order of creation/initialization

        if (!internal_data->dynamics_world) return;

        for (i32 i = internal_data->dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            remove_rigid_body(i);
        }

        delete internal_data->dynamics_world;
        delete internal_data->solver;
        delete internal_data->overlapping_pair_cache;
        delete internal_data->dispatcher;
        delete internal_data->collision_configuration;

        internal_data->dynamics_world = nullptr;
        internal_data->solver = nullptr;
        internal_data->overlapping_pair_cache = nullptr;
        internal_data->dispatcher = nullptr;
        internal_data->collision_configuration = nullptr;
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

        internal_data->dynamics_world->addRigidBody(bt_rigid_body);

        collider.internal = shape;
        rigid_body.internal = bt_rigid_body;
    }

    void PhysicsEngine::remove_rigid_body(const u32 index)
    {
        btCollisionObject* obj = internal_data->dynamics_world->getCollisionObjectArray()[index];
        btRigidBody* body = btRigidBody::upcast(obj);

        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }

        internal_data->dynamics_world->removeCollisionObject(obj);

        if (obj->getCollisionShape())
        {
            delete obj->getCollisionShape();
        }

        delete obj;
    }

    void PhysicsEngine::on_update(const f32 dt)
    {
        auto& ecs = scene->get_ecs();
        auto objects = ecs.get_all_components_of_types<TransformComponent, RigidBodyComponent>();

        if (!internal_data->dynamics_world) return;

        if (scene->is_running())
        {
            // @TODO: investigate the jittering that happens when maxSubSteps > 0.
            internal_data->dynamics_world->stepSimulation(dt, 0);

            for (i32 i = objects.size() - 1; i >= 0; i--)
            {
                auto [transform, rigid_body_c] = objects[i];
                auto* body = static_cast<btRigidBody*>(rigid_body_c->internal);

                btTransform trans(btQuaternion(0, 0, 0, 0));

                if (body && body->getMotionState())
                {
                    body->getMotionState()->getWorldTransform(trans);
                }

                else if (body)
                {
                    trans = body->getWorldTransform();
                }

                *transform = bt_transform_to_mag_transform(trans, transform->scale);
            }
        }

        // 'Draw' the debug lines before sending to the renderer
        physics_debug_draw->reset_lines();
        internal_data->dynamics_world->debugDrawWorld();
    }

    const LineList& PhysicsEngine::get_line_list() const { return physics_debug_draw->get_line_list(); };

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
