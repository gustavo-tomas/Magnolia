#pragma once

// Include this one before others
#include <Jolt/Jolt.h>
//
#include <Jolt/Core/Color.h>
#include <Jolt/Physics/Body/MotionProperties.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>

#include "magnolia/math/types.hpp"
#include "magnolia/physics/physics.hpp"

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

        inline JPH::EAllowedDOFs from_mag(const DegreesOfFreedom dof)
        {
            JPH::EAllowedDOFs jolt_dof = {};

            if (IS_BIT_SET(dof, DegreesOfFreedom::TranslationX))
            {
                jolt_dof |= JPH::EAllowedDOFs::TranslationX;
            }
            if (IS_BIT_SET(dof, DegreesOfFreedom::TranslationY))
            {
                jolt_dof |= JPH::EAllowedDOFs::TranslationY;
            }
            if (IS_BIT_SET(dof, DegreesOfFreedom::TranslationZ))
            {
                jolt_dof |= JPH::EAllowedDOFs::TranslationZ;
            }
            if (IS_BIT_SET(dof, DegreesOfFreedom::RotationX))
            {
                jolt_dof |= JPH::EAllowedDOFs::RotationX;
            }
            if (IS_BIT_SET(dof, DegreesOfFreedom::RotationY))
            {
                jolt_dof |= JPH::EAllowedDOFs::RotationY;
            }
            if (IS_BIT_SET(dof, DegreesOfFreedom::RotationZ))
            {
                jolt_dof |= JPH::EAllowedDOFs::RotationZ;
            }
            if (IS_BIT_SET(dof, DegreesOfFreedom::All))
            {
                jolt_dof |= JPH::EAllowedDOFs::All;
            }

            return jolt_dof;
        }

        inline DegreesOfFreedom to_mag(const JPH::EAllowedDOFs dof)
        {
            DegreesOfFreedom mag_dof = {};

            if (IS_BIT_SET(dof, JPH::EAllowedDOFs::TranslationX))
            {
                mag_dof |= DegreesOfFreedom::TranslationX;
            }
            if (IS_BIT_SET(dof, JPH::EAllowedDOFs::TranslationY))
            {
                mag_dof |= DegreesOfFreedom::TranslationY;
            }
            if (IS_BIT_SET(dof, JPH::EAllowedDOFs::TranslationZ))
            {
                mag_dof |= DegreesOfFreedom::TranslationZ;
            }
            if (IS_BIT_SET(dof, JPH::EAllowedDOFs::RotationX))
            {
                mag_dof |= DegreesOfFreedom::RotationX;
            }
            if (IS_BIT_SET(dof, JPH::EAllowedDOFs::RotationY))
            {
                mag_dof |= DegreesOfFreedom::RotationY;
            }
            if (IS_BIT_SET(dof, JPH::EAllowedDOFs::RotationZ))
            {
                mag_dof |= DegreesOfFreedom::RotationZ;
            }
            if (IS_BIT_SET(dof, JPH::EAllowedDOFs::All))
            {
                mag_dof |= DegreesOfFreedom::All;
            }

            return mag_dof;
        }
    };  // namespace physics
};  // namespace mag
