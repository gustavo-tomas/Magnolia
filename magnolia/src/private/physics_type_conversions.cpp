#include "private/physics_type_conversions.hpp"

#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"
#include "ecs/components.hpp"

namespace mag
{
    btTransform const mag_transform_to_bt_transform(const TransformComponent& t)
    {
        btTransform bt_transform;
        bt_transform.setIdentity();
        bt_transform.setOrigin(btVector3(t.translation.x, t.translation.y, t.translation.z));

        const quat mag_q(radians(t.rotation));
        const btQuaternion q(mag_q.x, mag_q.y, mag_q.z, mag_q.w);

        bt_transform.setRotation(q);

        return bt_transform;
    }

    TransformComponent const bt_transform_to_mag_transform(const btTransform& t, const vec3& scale)
    {
        TransformComponent transform;
        transform.translation = math::vec3(t.getOrigin().getX(), t.getOrigin().getY(), t.getOrigin().getZ());

        btScalar pitch, yaw, roll;
        t.getRotation().getEulerZYX(roll, yaw, pitch);
        transform.rotation = degrees(vec3(pitch, yaw, roll));

        // btTransform has no scale so we need to ask the user for that value
        transform.scale = scale;

        return transform;
    }

    btVector3 const mag_vec_to_bt_vec(const vec3& v)
    {
        const btVector3 bt_vec(v.x, v.y, v.z);

        return bt_vec;
    }

    vec3 const bt_vec_to_mag_vec(const btVector3& bt_vec)
    {
        const vec3 v(bt_vec.getX(), bt_vec.getY(), bt_vec.getZ());

        return v;
    }
};  // namespace mag