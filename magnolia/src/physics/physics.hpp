#pragma once

#include "btBulletDynamicsCommon.h"
#include "core/logger.hpp"
#include "ecs/components.hpp"
#include "scene/scene.hpp"

namespace mag
{
    class PhysicsDebugDraw;

    class PhysicsEngine
    {
        public:
            PhysicsEngine();
            ~PhysicsEngine();

            void on_simulation_start(Scene* scene);
            void on_simulation_end();

            void update(const f32 dt);

            const LineList& get_line_list() const;

        private:
            void add_rigid_body(const TransformComponent& transform, BoxColliderComponent& collider,
                                RigidBodyComponent& rigid_body);

            void remove_rigid_body(const u32 index);

            btDefaultCollisionConfiguration* collision_configuration = nullptr;
            btCollisionDispatcher* dispatcher = nullptr;
            btBroadphaseInterface* overlapping_pair_cache = nullptr;
            btSequentialImpulseConstraintSolver* solver = nullptr;
            btDiscreteDynamicsWorld* dynamics_world = nullptr;

            std::unique_ptr<PhysicsDebugDraw> physics_debug_draw;
            Scene* scene;
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

            virtual void reportErrorWarning(const char* warning_string) override { LOG_ERROR("{0}", warning_string); }

            virtual void draw3dText(const btVector3& location, const char* text_string) override
            {
                (void)location;
                LOG_ERROR("3D text not supported: {0}", text_string);
            }

            virtual void setDebugMode(int debugMode) override { (void)debugMode; }  // @TODO: finish debug mode
            virtual int getDebugMode() const override { return btIDebugDraw::DBG_DrawWireframe; }

        private:
            LineList line_list;
    };
};  // namespace mag
