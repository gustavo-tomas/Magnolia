#pragma once

// Include this one before others
#include <Jolt/Jolt.h>
//
#include <Jolt/Core/Color.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>

#include "magnolia/core/assert.hpp"
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

        inline void from_mag(const std::vector<math::Triangle>& triangles, JPH::TriangleList& out_triangles)
        {
            out_triangles.resize(triangles.size());

            for (u64 i = 0; i < out_triangles.size(); i++)
            {
                out_triangles[i].mV[0].x = triangles[i].v0.x;
                out_triangles[i].mV[0].y = triangles[i].v0.y;
                out_triangles[i].mV[0].z = triangles[i].v0.z;

                out_triangles[i].mV[1].x = triangles[i].v1.x;
                out_triangles[i].mV[1].y = triangles[i].v1.y;
                out_triangles[i].mV[1].z = triangles[i].v1.z;

                out_triangles[i].mV[2].x = triangles[i].v2.x;
                out_triangles[i].mV[2].y = triangles[i].v2.y;
                out_triangles[i].mV[2].z = triangles[i].v2.z;
            }
        }
    };  // namespace physics
};  // namespace mag
