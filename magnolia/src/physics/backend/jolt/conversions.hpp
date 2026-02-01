#pragma once

// Include this one before others
#include <Jolt/Jolt.h>
//
#include <Jolt/Core/Color.h>

#include "magnolia/math/types.hpp"

namespace mag
{
    namespace physics
    {
        inline JPH::Vec3 from_mag(const math::vec3& v)
        {
            const JPH::Vec3 vec(v.x, v.y, v.z);

            return vec;
        }

        inline math::vec3 to_mag(const JPH::Vec3& vec)
        {
            const math::vec3 v(vec.GetX(), vec.GetY(), vec.GetZ());

            return v;
        }

        inline math::vec4 to_mag(const JPH::Color& col)
        {
            const math::vec4 c(col.r, col.g, col.b, col.a);

            return c;
        }

        inline JPH::Quat from_mag(const math::quat& q)
        {
            const JPH::Quat quat(q.x, q.y, q.z, q.w);

            return quat;
        }

        inline math::quat to_mag(const JPH::Quat& quat)
        {
            const math::quat q(quat.GetW(), quat.GetX(), quat.GetY(), quat.GetZ());

            return q;
        }
    };  // namespace physics
};  // namespace mag
