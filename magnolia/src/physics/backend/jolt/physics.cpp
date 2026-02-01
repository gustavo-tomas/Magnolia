#include "magnolia/physics/physics.hpp"

#include "magnolia/core/types.hpp"
#include "magnolia/threads/thread.hpp"
#include "physics/backend/jolt/conversions.hpp"
#include "physics/backend/jolt/implementations.hpp"

// Include this one before others
#include <Jolt/Jolt.h>
//
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

namespace mag
{
    namespace physics
    {
        // Jolt uses a singleton factory and because of that we have to implement some extra logic to prevent the
        // instance from being created twice when a new world is created. We can however create different physics
        // systems.

        struct State
        {
                unique<DebugRenderer> debug_renderer = nullptr;

                // @TODO: this is an example job system. Integrate jolt with our job system.
                unique<JPH::JobSystemThreadPool> job_system = nullptr;

                // We need a temp allocator for temporary allocations during the physics update
                unique<JPH::TempAllocatorImpl> temp_allocator = nullptr;
        };

        static State* state = nullptr;

        b8 initialize()
        {
            state = new State();

            // Register allocation hook.
            // This needs to be done before any other Jolt function is called.
            JPH::RegisterDefaultAllocator();

            // Install trace and assert callbacks
            JPH::Trace = trace_callback;
            JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = assert_failed_callback;)

            // Create a factory, this class is responsible for creating instances of classes based on their name
            // or hash and is mainly used for deserialization of saved data.
            JPH::Factory::sInstance = new JPH::Factory();
            state->debug_renderer = create_unique<DebugRenderer>();

            // Register all physics types with the factory and install their collision handlers with the
            // CollisionDispatch class.
            JPH::RegisterTypes();

            const u64 allocation_size = 1ULL * 20 * 1024 * 1024;
            physics::state->temp_allocator = create_unique<JPH::TempAllocatorImpl>(allocation_size);

            physics::state->job_system = create_unique<JPH::JobSystemThreadPool>(
                JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, static_cast<i32>(thread::get_core_count() - 1));

            return state != nullptr;
        }

        void shutdown()
        {
            // Unregisters all types with the factory and cleans up the default material
            JPH::UnregisterTypes();

            // Destroy the factory
            delete JPH::Factory::sInstance;
            JPH::Factory::sInstance = nullptr;

            delete state;
        }

        class JoltPhysicsWorld : public IPhysicsWorld
        {
            public:
                JoltPhysicsWorld()
                {
                    // Max number of bodies in the simulation
                    const u32 max_bodies = 0xFFFFF;

                    // Max number of mutexes for rigid bodies. 0 is the default setting.
                    const u32 num_body_mutexes = 0;

                    // This is the max amount of body pairs that can be queued at any time.
                    const u32 max_body_pairs = 0xFFFFF;

                    // This is the maximum size of the contact constraint buffer. If more contacts (collisions between
                    // bodies) are detected than this number then these contacts will be ignored and bodies will start
                    // interpenetrating / fall through the world.
                    const u32 max_contact_constraints = 10240;

                    // Create mapping table from object layer to broadphase layer
                    broad_phase_layer_interface = create_unique<BPLayerInterfaceImpl>();

                    // Create class that filters object vs broadphase layers
                    object_vs_broadphase_layer_filter = create_unique<ObjectVsBroadPhaseLayerFilterImpl>();

                    // Create class that filters object vs object layers
                    object_vs_object_layer_filter = create_unique<ObjectLayerPairFilterImpl>();

                    // Now we can create the actual physics system.
                    physics_system = create_unique<JPH::PhysicsSystem>();
                    physics_system->Init(max_bodies, num_body_mutexes, max_body_pairs, max_contact_constraints,
                                         *broad_phase_layer_interface, *object_vs_broadphase_layer_filter,
                                         *object_vs_object_layer_filter);

                    body_activation_listener = create_unique<BodyActivationListener>();

                    contact_listener = create_unique<ContactListener>();

                    physics_system->SetBodyActivationListener(body_activation_listener.get());

                    physics_system->SetContactListener(contact_listener.get());
                }

                ~JoltPhysicsWorld() override
                {
                    JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    for (const auto& [id, body_id] : rigid_bodies)
                    {
                        body_interface.RemoveBody(body_id);
                        body_interface.DestroyBody(body_id);
                    }
                }

                static RigidBodyHandle create_handle()
                {
                    static RigidBodyHandle handle_counter = 0;

                    return handle_counter++;
                }

                RigidBodyHandle add_rigid_body_base(const JPH::ShapeSettings::ShapeResult& shape_result,
                                                    const math::vec3& position, const math::quat& rotation,
                                                    const f32 mass)
                {
                    const RigidBodyHandle handle = create_handle();

                    // The main way to interact with the bodies in the physics system is through the body interface.
                    // There is a locking and a non-locking variant of this. We're going to use the locking version
                    // (even though we're not planning to access bodies from multiple threads)
                    JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    if (shape_result.HasError())
                    {
                        LOG_ERROR("[Physics] Error when adding rigidbody: '{0}'", shape_result.GetError().c_str());
                        return handle;
                    }

                    const JPH::ShapeRefC shape = shape_result.Get();
                    const JPH::RVec3 pos = physics::from_mag(position);
                    const JPH::Quat rot = physics::from_mag(rotation);

                    // @TODO: also check kinematic motion type
                    const JPH::EMotionType motion_type =
                        (mass > 0.0f) ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static;

                    JPH::BodyCreationSettings creation_settings(shape, pos, rot, motion_type, Layers::Moving);

                    JPH::MassProperties mass_properties;
                    mass_properties.ScaleToMass(mass);

                    creation_settings.mMassPropertiesOverride = mass_properties;
                    creation_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;

                    // Create the actual rigid body
                    // Note that if we run out of bodies this can return nullptr
                    const JPH::Body* body = body_interface.CreateBody(creation_settings);

                    // Add it to the world
                    MAG_ASSERT(body != nullptr, "[Physics] Failed to create rigidbody");
                    if (body == nullptr)
                    {
                        return handle;
                    }

                    body_interface.AddBody(body->GetID(), JPH::EActivation::Activate);
                    rigid_bodies[handle] = body->GetID();

                    // Optimize the broad phase after adding N objects
                    constexpr u32 Body_Batch_Size = 10;

                    if (rigid_bodies.size() % Body_Batch_Size == 0)
                    {
                        // Before starting the physics simulation you can optimize the broad phase. This improves
                        // collision detection performance. You should definitely not call this every frame or when e.g.
                        // streaming in a new level section as it is an expensive operation. Instead insert all new
                        // objects in batches instead of 1 at a time to keep the broad phase efficient.
                        physics_system->OptimizeBroadPhase();
                    }

                    return handle;
                }

                RigidBodyHandle add_rigid_body(const math::vec3& position, const math::quat& rotation,
                                               const math::vec3& collider_dimensions, const f32 mass) override
                {
                    // Create the settings for the collision volume (the shape).
                    const JPH::BoxShapeSettings shape_settings(physics::from_mag(collider_dimensions));

                    // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it
                    // from being freed when its reference count goes to 0.
                    shape_settings.SetEmbedded();

                    // Create the shape
                    const JPH::ShapeSettings::ShapeResult shape_result = shape_settings.Create();

                    const RigidBodyHandle handle = add_rigid_body_base(shape_result, position, rotation, mass);

                    return handle;
                }

                RigidBodyHandle add_rigid_body(const math::vec3& position, const math::quat& rotation, const f32 radius,
                                               const f32 height, const f32 mass) override
                {
                    const JPH::CapsuleShapeSettings shape_settings(height, radius);

                    shape_settings.SetEmbedded();

                    const JPH::ShapeSettings::ShapeResult shape_result = shape_settings.Create();

                    const RigidBodyHandle handle = add_rigid_body_base(shape_result, position, rotation, mass);

                    return handle;
                }

                void remove_rigid_body(const RigidBodyHandle handle) override
                {
                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return;
                    }

                    JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    const JPH::BodyID id = it->second;

                    body_interface.RemoveBody(id);
                    body_interface.DestroyBody(id);

                    rigid_bodies.erase(it);
                }

                // Can't go wrong with this one
                // https://gafferongames.com/post/fix_your_timestep/

                // @TODO: this still isn't perfect and some stutter will happen after a few frames. For now we keep the
                // application running in a steady timestep and maybe we will solve this later.

                void on_update(const f32 dt) override
                {
                    // We simulate the physics world in discrete time steps. 60 Hz is a good rate to update the physics
                    // system.
                    const f32 fixed_dt = 1.0f / 60.0f;
                    static f32 accumulated_dt = 0.0f;

                    // Adjust collision steps according to the fixed dt
                    i32 collision_steps = 0;

                    // Cap the collision steps (if we ever hit this stage it will be a miserable experience anyways)
                    accumulated_dt += dt;
                    accumulated_dt = math::min(accumulated_dt, 1.0f / 4.0f);

                    while (accumulated_dt >= fixed_dt)
                    {
                        collision_steps++;
                        accumulated_dt -= fixed_dt;
                    }

                    if (collision_steps > 0)
                    {
                        // Step the world
                        physics_system->Update(fixed_dt, collision_steps, physics::state->temp_allocator.get(),
                                               physics::state->job_system.get());
                    }

                    // Render the world
                    static JPH::BodyManager::DrawSettings settings = {};
                    settings.mDrawShape = true;
                    settings.mDrawShapeWireframe = true;

                    physics::state->debug_renderer->reset_line_list();
                    physics_system->DrawBodies(settings, physics::state->debug_renderer.get());
                }

                void apply_force(const RigidBodyHandle handle, const math::vec3& force) override
                {
                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return;
                    }

                    JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    body_interface.AddForce(it->second, physics::from_mag(force));
                }

                void apply_impulse(const RigidBodyHandle handle, const math::vec3& impulse) override
                {
                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return;
                    }

                    JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    body_interface.AddImpulse(it->second, physics::from_mag(impulse));
                }

                void apply_torque(const RigidBodyHandle handle, const math::vec3& torque) override
                {
                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return;
                    }

                    JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    body_interface.AddTorque(it->second, physics::from_mag(torque));
                }

                void apply_torque_impulse(const RigidBodyHandle handle, const math::vec3& torque) override
                {
                    (void)torque;

                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return;
                    }

                    // @TODO: idk
                }

                void set_linear_velocity(const RigidBodyHandle handle, const math::vec3& velocity) override
                {
                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return;
                    }

                    JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    body_interface.SetLinearVelocity(it->second, physics::from_mag(velocity));
                }

                void set_angular_velocity(const RigidBodyHandle handle, const math::vec3& velocity) override
                {
                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return;
                    }

                    JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    body_interface.SetAngularVelocity(it->second, physics::from_mag(velocity));
                }

                void set_angular_factor(const RigidBodyHandle handle, const math::vec3& axes) override
                {
                    (void)axes;

                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return;
                    }

                    // @TODO: idk
                }

                void set_activation_state(const RigidBodyHandle handle, const ActivationState activation_state) override
                {
                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return;
                    }

                    JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    switch (activation_state)
                    {
                        case ActivationState::Activate:
                            body_interface.ActivateBody(it->second);
                            break;

                        case ActivationState::Deactivate:
                            body_interface.DeactivateBody(it->second);
                            break;

                        default:
                            MAG_ASSERT(false, "[Physics] Invalid activation state");
                            break;
                    }
                }

                math::vec3 get_linear_velocity(const RigidBodyHandle handle) const override
                {
                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return math::vec3(0.0f);
                    }

                    const JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    const JPH::Vec3 velocity = body_interface.GetLinearVelocity(it->second);

                    return physics::to_mag(velocity);
                }

                math::vec3 get_angular_velocity(const RigidBodyHandle handle) const override
                {
                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return math::vec3(0.0f);
                    }

                    const JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    const JPH::Vec3 velocity = body_interface.GetAngularVelocity(it->second);

                    return physics::to_mag(velocity);
                }

                void get_collision_object_transform(const RigidBodyHandle handle, math::vec3& position,
                                                    math::quat& rotation) const override
                {
                    auto it = rigid_bodies.find(handle);
                    if (it == rigid_bodies.end())
                    {
                        MAG_ASSERT(false, "Invalid rigidbody handle");
                        return;
                    }

                    const JPH::BodyInterface& body_interface = physics_system->GetBodyInterface();

                    const JPH::BodyID& id = it->second;
                    JPH::RVec3 pos = {};
                    JPH::Quat rot = {};
                    body_interface.GetPositionAndRotation(id, pos, rot);

                    position = physics::to_mag(pos);
                    rotation = physics::to_mag(rot);
                }

                const math::LineList& get_debug_line_list() const override
                {
                    return physics::state->debug_renderer->get_line_list();
                }

            private:
                unique<JPH::PhysicsSystem> physics_system = nullptr;

                // Create mapping table from object layer to broadphase layer
                unique<BPLayerInterfaceImpl> broad_phase_layer_interface = nullptr;

                // Create class that filters object vs broadphase layers
                unique<ObjectVsBroadPhaseLayerFilterImpl> object_vs_broadphase_layer_filter = nullptr;

                // Create class that filters object vs object layers
                unique<ObjectLayerPairFilterImpl> object_vs_object_layer_filter = nullptr;

                // A body activation listener gets notified when bodies activate and go to sleep
                // Note that this is called from a job so whatever you do here needs to be thread safe.
                // Registering one is entirely optional.
                unique<BodyActivationListener> body_activation_listener = nullptr;

                // A contact listener gets notified when bodies (are about to) collide, and when they separate again.
                // Note that this is called from a job so whatever you do here needs to be thread safe.
                // Registering one is entirely optional.
                unique<ContactListener> contact_listener = nullptr;

                std::unordered_map<RigidBodyHandle, JPH::BodyID> rigid_bodies;
        };

        unique<IPhysicsWorld> create_physics_world() { return create_unique<JoltPhysicsWorld>(); }
    };  // namespace physics
};  // namespace mag
