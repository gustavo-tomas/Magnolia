#pragma once

#include "magnolia/core/assert.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"
#include "physics/backend/jolt/conversions.hpp"

// Include this one before others
#include <Jolt/Jolt.h>
//
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Renderer/DebugRenderer.h>

#include <cstdarg>
#include <unordered_set>

// See the hello world example for nice explanations of the jolt structures
// https://github.com/jrouwe/JoltPhysics/blob/master/HelloWorld/HelloWorld.cpp

// Disable common warnings triggered by Jolt, you can use JPH_SUPPRESS_WARNING_PUSH / JPH_SUPPRESS_WARNING_POP to store
// and restore the warning state
// JPH_SUPPRESS_WARNINGS

namespace mag
{
    namespace physics
    {
        static void trace_callback(const c8* fmt, ...)
        {
            va_list list;
            va_start(list, fmt);
            std::array<c8, 1024> buffer = {};
            vsnprintf(buffer.data(), sizeof(buffer), fmt, list);
            va_end(list);

            LOG_INFO("[Physics] {}", buffer.data());
        }

#ifdef JPH_ENABLE_ASSERTS

        static b8 assert_failed_callback(const c8* expression, const c8* message, const c8* file, u32 line)
        {
            MAG_ASSERT(false, "[Physics] {0}:{1}: ({2}) {3}", file, line, expression,
                       (message != nullptr ? message : ""));

            return true;
        }

#endif  // JPH_ENABLE_ASSERTS

        class DebugRenderer : public JPH::DebugRenderer
        {
            public:
                DebugRenderer() { Initialize(); }

                void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override
                {
                    math::Line line = {};
                    line.start = to_mag(inFrom);
                    line.end = to_mag(inTo);
                    line.color = math::vec3(to_mag(inColor));

                    line_list.append(line);
                }

                void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor,
                                  ECastShadow inCastShadow) override
                {
                    (void)inV1;
                    (void)inV2;
                    (void)inV3;
                    (void)inColor;
                    (void)inCastShadow;

                    LOG_WARNING("[Physics] @TODO: TRIANGLE DRAW");
                }

                void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, f32 inLODScaleSq,
                                  JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode,
                                  ECastShadow inCastShadow, EDrawMode inDrawMode) override
                {
                    (void)inLODScaleSq;
                    (void)inCullMode;
                    (void)inWorldSpaceBounds;

                    // Figure out which LOD to use
                    // We don't care much about detail, so just get the lowest lod possible
                    const LOD* lod = &inGeometry->mLODs.back();
                    // lod = &inGeometry->GetLOD(camera_position, inWorldSpaceBounds, inLODScaleSq);

                    // Draw the batch
                    const auto* batch = dynamic_cast<const BatchImpl*>(lod->mTriangleBatch.GetPtr());
                    if (batch == nullptr)
                    {
                        return;
                    }

                    for (const Triangle& triangle : batch->triangles)
                    {
                        const JPH::RVec3 v0 = inModelMatrix * JPH::Vec3(triangle.mV[0].mPosition);
                        const JPH::RVec3 v1 = inModelMatrix * JPH::Vec3(triangle.mV[1].mPosition);
                        const JPH::RVec3 v2 = inModelMatrix * JPH::Vec3(triangle.mV[2].mPosition);
                        const JPH::Color color = inModelColor * triangle.mV[0].mColor;

                        switch (inDrawMode)
                        {
                            case EDrawMode::Wireframe:
                                DrawLine(v0, v1, color);
                                DrawLine(v1, v2, color);
                                DrawLine(v2, v0, color);
                                break;

                            case EDrawMode::Solid:
                                DrawTriangle(v0, v1, v2, color, inCastShadow);
                                break;
                        }
                    }
                }

                void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor,
                                f32 inHeight) override
                {
                    (void)inPosition;
                    (void)inString;
                    (void)inColor;
                    (void)inHeight;

                    LOG_WARNING("[Physics] @TODO: TEXT DRAW");
                }

                Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override
                {
                    auto* batch = new BatchImpl();
                    if (inTriangles == nullptr || inTriangleCount == 0)
                    {
                        return batch;
                    }

                    batch->triangles.assign(inTriangles, inTriangles + inTriangleCount);

                    return batch;
                }

                Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const u32* inIndices,
                                          int inIndexCount) override
                {
                    auto* batch = new BatchImpl();
                    if (inVertices == nullptr || inVertexCount == 0 || inIndices == nullptr || inIndexCount == 0)
                    {
                        return batch;
                    }

                    // Convert indexed triangle list to triangle list
                    batch->triangles.resize(inIndexCount / 3);
                    for (u64 t = 0; t < batch->triangles.size(); ++t)
                    {
                        Triangle& triangle = batch->triangles[t];
                        triangle.mV[0] = inVertices[inIndices[(t * 3) + 0]];
                        triangle.mV[1] = inVertices[inIndices[(t * 3) + 1]];
                        triangle.mV[2] = inVertices[inIndices[(t * 3) + 2]];
                    }

                    return batch;
                }

                const math::LineList& get_line_list() const { return line_list; }

                void reset_line_list() { line_list.lines.clear(); }

            private:
                math::LineList line_list = {};

                class BatchImpl : public JPH::RefTargetVirtual
                {
                    public:
                        void AddRef() override { ++ref_count; }

                        void Release() override
                        {
                            if (--ref_count == 0)
                            {
                                delete this;
                            }
                        }

                        JPH::Array<Triangle> triangles;

                    private:
                        std::atomic<u32> ref_count = 0;
                };
        };

        // Layer that objects can be in, determines which other objects it can collide with
        // Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have
        // more layers if you want. E.g. you could have a layer for high detail collision (which is not used by the
        // physics simulation but only if you do collision testing).
        namespace Layers
        {
            static constexpr JPH::ObjectLayer Non_Moving = 0;
            static constexpr JPH::ObjectLayer Moving = 1;
            static constexpr JPH::ObjectLayer Num_Layers = 2;
        };  // namespace Layers

        /// Class that determines if two object layers can collide
        class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
        {
            public:
                b8 ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const override
                {
                    switch (object1)
                    {
                        case Layers::Non_Moving:
                            return object2 == Layers::Moving;  // Non moving only collides with moving

                        case Layers::Moving:
                            return true;  // Moving collides with everything

                        default:
                            MAG_ASSERT(false, "[Physics] Unspecified object collision");
                            return false;
                    }
                }
        };

        // Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to
        // have a layer for non-moving and moving objects to avoid having to update a tree full of static objects every
        // frame. You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if
        // you have many object layers you'll be creating many broad phase trees, which is not efficient. If you want to
        // fine tune your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
        namespace BroadPhaseLayers
        {
            static constexpr JPH::BroadPhaseLayer Non_Moving(0);
            static constexpr JPH::BroadPhaseLayer Moving(1);
            static constexpr u32 Num_Layers(2);
        };  // namespace BroadPhaseLayers

        // This defines a mapping between object and broadphase layers.
        class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
        {
            public:
                BPLayerInterfaceImpl()
                {
                    // Create a mapping table from object to broad phase layer
                    object_to_broad_phase[Layers::Non_Moving] = BroadPhaseLayers::Non_Moving;
                    object_to_broad_phase[Layers::Moving] = BroadPhaseLayers::Moving;
                }

                u32 GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::Num_Layers; }

                JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
                {
                    MAG_ASSERT(layer < Layers::Num_Layers, "[Physics] Layer count exceeded");
                    return object_to_broad_phase.at(layer);
                }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
                const c8* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
                {
                    switch (static_cast<JPH::BroadPhaseLayer::Type>(layer))
                    {
                        case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::Non_Moving):
                            return "NON_MOVING";

                        case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::Moving):
                            return "MOVING";

                        default:
                            MAG_ASSERT(false, "[Physics] Invalid broad phase layer name");
                            return "INVALID";
                    }
                }
#endif  // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

            private:
                std::array<JPH::BroadPhaseLayer, Layers::Num_Layers> object_to_broad_phase = {};
        };

        /// Class that determines if an object layer can collide with a broadphase layer
        class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
        {
            public:
                b8 ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override
                {
                    switch (layer1)
                    {
                        case Layers::Non_Moving:
                            return layer2 == BroadPhaseLayers::Moving;

                        case Layers::Moving:
                            return true;

                        default:
                            MAG_ASSERT(false, "[Physics] Invalid collision settings");
                            return false;
                    }
                }
        };

        // Listeners

        class ContactListener : public JPH::ContactListener
        {
            public:
                // See: JPH::ContactListener
                JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                                      JPH::RVec3Arg inBaseOffset,
                                                      const JPH::CollideShapeResult& inCollisionResult) override
                {
                    (void)inBody1;
                    (void)inBody2;
                    (void)inBaseOffset;
                    (void)inCollisionResult;

                    // Allows you to ignore a contact before it is created (using layers to not make objects collide is
                    // cheaper!)
                    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
                }

                void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                    const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
                {
                    (void)inManifold;
                    (void)ioSettings;

                    const JPH::BodyID& body_1_id = inBody1.GetID();
                    const JPH::BodyID& body_2_id = inBody2.GetID();

                    contacts[body_1_id].insert(body_2_id);
                    contacts[body_2_id].insert(body_1_id);
                }

                void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2,
                                        const JPH::ContactManifold& inManifold,
                                        JPH::ContactSettings& ioSettings) override
                {
                    (void)inBody1;
                    (void)inBody2;
                    (void)inManifold;
                    (void)ioSettings;
                }

                void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
                {
                    const JPH::BodyID& body_1_id = inSubShapePair.GetBody1ID();
                    const JPH::BodyID& body_2_id = inSubShapePair.GetBody2ID();

                    contacts[body_1_id].erase(body_2_id);
                    contacts[body_2_id].erase(body_1_id);
                }

                const std::unordered_map<JPH::BodyID, std::unordered_set<JPH::BodyID>>& get_active_contacts() const
                {
                    return contacts;
                }

            private:
                std::unordered_map<JPH::BodyID, std::unordered_set<JPH::BodyID>> contacts;
        };

        class BodyActivationListener : public JPH::BodyActivationListener
        {
            public:
                void OnBodyActivated(const JPH::BodyID& inBodyID, u64 inBodyUserData) override
                {
                    (void)inBodyID;
                    (void)inBodyUserData;
                }

                void OnBodyDeactivated(const JPH::BodyID& inBodyID, u64 inBodyUserData) override
                {
                    (void)inBodyID;
                    (void)inBodyUserData;
                }
        };

        // Collectors

        class CollideShapeCollector : public JPH::CollideShapeCollector
        {
            public:
                const std::vector<JPH::BodyID>& get_collisions() const { return collisions; }

                void AddHit(const JPH::CollideShapeResult& inResult) override
                {
                    collisions.push_back(inResult.mBodyID2);
                }

                std::vector<JPH::BodyID> collisions;
        };
    };  // namespace physics
};  // namespace mag
