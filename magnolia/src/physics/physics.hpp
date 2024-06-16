#pragma once

#include <vector>

#include "btBulletDynamicsCommon.h"
#include "core/logger.hpp"
#include "renderer/model.hpp"

namespace mag
{
    struct BulletCollisionShape
    {
            BulletCollisionShape(btCollisionShape* shape) : shape(shape) {}

            btCollisionShape* shape;
    };

    struct BulletRigidBody
    {
            BulletRigidBody(btRigidBody* rigid_body, const f32 mass) : rigid_body(rigid_body), mass(mass) {}

            btRigidBody* rigid_body;
            f32 mass;
    };

    class PhysicsDebugDraw;

    class PhysicsEngine
    {
        public:
            PhysicsEngine();
            ~PhysicsEngine();

            BulletCollisionShape* create_collision_shape(const vec3& box_half_extents);

            BulletRigidBody* create_rigid_body(const BulletCollisionShape& shape, const TransformComponent& transform,
                                               const f32 mass);

            void update(const f32 dt);

            const std::unique_ptr<Line>& get_line_list() const;

        private:
            btDefaultCollisionConfiguration* collision_configuration;
            btCollisionDispatcher* dispatcher;
            btBroadphaseInterface* overlapping_pair_cache;
            btSequentialImpulseConstraintSolver* solver;
            btDiscreteDynamicsWorld* dynamics_world;

            PhysicsDebugDraw* physics_debug_draw;
    };

    // @TODO: finish debug draw
    class PhysicsDebugDraw : public btIDebugDraw
    {
        public:
            void create_lines();
            void reset_lines();

            const std::unique_ptr<Line>& get_line_list() const { return line_list; };

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

            virtual void reportErrorWarning(const char* warning_string) override { LOG_ERROR("{0}", warning_string); }

            virtual void draw3dText(const btVector3& location, const char* text_string) override
            {
                (void)location;
                LOG_ERROR("3D text not supported: {0}", text_string);
            }

            virtual void setDebugMode(int debugMode) override { (void)debugMode; }  // @TODO: finish debug mode
            virtual int getDebugMode() const override { return btIDebugDraw::DBG_DrawWireframe; }

        private:
            std::unique_ptr<Line> line_list;
            std::vector<vec3> starts, ends, colors;
    };
};  // namespace mag
