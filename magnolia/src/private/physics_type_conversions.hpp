#pragma once

#include "core/math.hpp"

class btTransform;
class btVector3;

namespace mag
{
    struct TransformComponent;

    btTransform const mag_transform_to_bt_transform(const TransformComponent& t);
    TransformComponent const bt_transform_to_mag_transform(const btTransform& t, const math::vec3& scale);
    btVector3 const mag_vec_to_bt_vec(const math::vec3& v);
    math::vec3 const bt_vec_to_mag_vec(const btVector3& bt_vec);
};  // namespace mag
