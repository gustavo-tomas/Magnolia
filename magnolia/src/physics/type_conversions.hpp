#pragma once

#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"
#include "ecs/components.hpp"

namespace mag
{
    inline btTransform const mag_transform_to_bt_transform(const TransformComponent& t)
    {
        btTransform bt_transform;
        bt_transform.setIdentity();
        bt_transform.setOrigin(btVector3(t.translation.x, t.translation.y, t.translation.z));

        btQuaternion q(radians(t.rotation.y), radians(t.rotation.x), radians(t.rotation.z));
        bt_transform.setRotation(q);

        return bt_transform;
    }

    inline TransformComponent const bt_transform_to_mag_transform(const btTransform& t, const vec3& scale)
    {
        TransformComponent transform;
        transform.translation = vec3(t.getOrigin().getX(), t.getOrigin().getY(), t.getOrigin().getZ());

        btScalar pitch, yaw, roll;
        t.getRotation().getEulerZYX(roll, yaw, pitch);
        transform.rotation = vec3(degrees(pitch), degrees(yaw), degrees(roll));

        // btTransform has no scale so we need to ask the user for that value
        transform.scale = scale;

        return transform;
    }

    inline btVector3 const mag_vec_to_bt_vec(const vec3& v)
    {
        btVector3 bt_vec(v.x, v.y, v.z);

        return bt_vec;
    }

    inline vec3 const bt_vec_to_mag_vec(const btVector3& bt_vec)
    {
        vec3 v(bt_vec.getX(), bt_vec.getY(), bt_vec.getZ());

        return v;
    }
};  // namespace mag
