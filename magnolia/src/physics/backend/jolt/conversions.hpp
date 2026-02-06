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

        inline void from_mag(const std::vector<math::vec3>& vertices, JPH::VertexList& out_vertices)
        {
            out_vertices.resize(vertices.size());

            for (u64 i = 0; i < vertices.size(); i++)
            {
                out_vertices[i].x = vertices[i].x;
                out_vertices[i].y = vertices[i].y;
                out_vertices[i].z = vertices[i].z;
            }
        }

        inline void from_mag(const std::vector<u32>& indices, JPH::IndexedTriangleList& out_indices)
        {
            MAG_ASSERT(indices.size() % 3 == 0, "Invalid index list, size must be divisible by 3: {0}", indices.size());

            out_indices.resize(indices.size() / 3);

            for (u64 i = 0; i < indices.size(); i += 3)
            {
                out_indices[i].mIdx[0] = indices[i];
                out_indices[i].mIdx[1] = indices[i + 1];
                out_indices[i].mIdx[2] = indices[i + 2];
            }
        }
    };  // namespace physics
};  // namespace mag
