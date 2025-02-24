#pragma once

#include "math/types.hpp"

class btTransform;
class btVector3;

namespace mag
{
    struct TransformComponent;

    btTransform const mag_transform_to_bt_transform(const TransformComponent& t);
    btTransform const mag_transform_to_bt_transform(const math::vec3& position, const math::quat& rotation);
    TransformComponent const bt_transform_to_mag_transform(const btTransform& t);
    btVector3 const mag_vec_to_bt_vec(const math::vec3& v);
    math::vec3 const bt_vec_to_mag_vec(const btVector3& bt_vec);
};  // namespace mag
