#pragma once

#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"
#include "magnolia/core/assert.hpp"
#include "magnolia/ecs/components.hpp"
#include "magnolia/math/types.hpp"
#include "magnolia/physics/physics.hpp"

namespace mag
{
    inline btVector3 const mag_to_bt(const vec3& v)
    {
        const btVector3 bt_vec(v.x, v.y, v.z);

        return bt_vec;
    }

    inline vec3 const bt_to_mag(const btVector3& bt_vec)
    {
        const vec3 v(bt_vec.getX(), bt_vec.getY(), bt_vec.getZ());

        return v;
    }

    inline btTransform const mag_to_bt(const TransformComponent& t)
    {
        btTransform bt_transform;
        bt_transform.setIdentity();
        bt_transform.setOrigin(btVector3(t.translation.x, t.translation.y, t.translation.z));

        const quat mag_q(t.rotation);
        const btQuaternion q(mag_q.x, mag_q.y, mag_q.z, mag_q.w);

        bt_transform.setRotation(q);

        return bt_transform;
    }

    inline btTransform const mag_to_bt(const math::vec3& position, const math::quat& rotation)
    {
        btTransform bt_transform;
        bt_transform.setIdentity();
        bt_transform.setOrigin(mag_to_bt(position));

        const btQuaternion q(rotation.x, rotation.y, rotation.z, rotation.w);

        bt_transform.setRotation(q);

        return bt_transform;
    }

    inline TransformComponent const bt_to_mag(const btTransform& t)
    {
        TransformComponent transform;
        transform.translation = math::vec3(t.getOrigin().getX(), t.getOrigin().getY(), t.getOrigin().getZ());

        btScalar pitch, yaw, roll;
        t.getRotation().getEulerZYX(roll, yaw, pitch);
        transform.rotation = vec3(pitch, yaw, roll);

        return transform;
    }

    inline i32 mag_to_bt(const ActivationState state)
    {
        switch (state)
        {
            case ActivationState::DisableDeactivation:
                return DISABLE_DEACTIVATION;
                break;

            default:
                MAG_ASSERT(false, "Unhandled activation state");
                return 0;
                break;
        }
    }

    inline ActivationState bt_to_mag(const i32 state)
    {
        switch (state)
        {
            case DISABLE_DEACTIVATION:
                return ActivationState::DisableDeactivation;
                break;

            default:
                MAG_ASSERT(false, "Unhandled activation state");
                return ActivationState::DisableDeactivation;
                break;
        }
    }
};  // namespace mag
