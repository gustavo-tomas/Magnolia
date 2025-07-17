#pragma once

#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"
#include "magnolia/core/assert.hpp"
#include "magnolia/math/types.hpp"
#include "magnolia/physics/physics.hpp"

namespace mag
{
    inline btVector3 const mag_to_bt(const math::vec3& v)
    {
        const btVector3 bt_vec(v.x, v.y, v.z);

        return bt_vec;
    }

    inline math::vec3 const bt_to_mag(const btVector3& bt_vec)
    {
        const math::vec3 v(bt_vec.x(), bt_vec.y(), bt_vec.z());

        return v;
    }

    inline btQuaternion const mag_to_bt(const math::quat& q)
    {
        const btQuaternion bt_quat(q.x, q.y, q.z, q.w);

        return bt_quat;
    }

    inline math::quat const bt_to_mag(const btQuaternion& bt_quat)
    {
        const math::quat q(bt_quat.w(), bt_quat.x(), bt_quat.y(), bt_quat.z());

        return q;
    }

    inline btTransform const mag_to_bt(const math::vec3& position, const math::quat& rotation)
    {
        btTransform bt_transform;
        bt_transform.setIdentity();
        bt_transform.setOrigin(mag_to_bt(position));
        bt_transform.setRotation(mag_to_bt(rotation));

        return bt_transform;
    }

    inline void bt_to_mag(const btTransform& t, math::vec3& translation, math::quat& rotation)
    {
        math::mat4 mag_transform = math::mat4(1.0f);

        t.getOpenGLMatrix(math::value_ptr(mag_transform));

        math::vec3 scale;
        math::decompose_simple(mag_transform, scale, rotation, translation);
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
