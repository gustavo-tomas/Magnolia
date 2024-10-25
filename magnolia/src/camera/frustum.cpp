#include "camera/frustum.hpp"

namespace mag
{
    enum Planes
    {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far,
        Count,
        Combinations = Count * (Count - 1) / 2
    };

    template <Planes i, Planes j>
    struct ij2k
    {
            enum
            {
                k = i * (9 - i) / 2 + j - 1
            };
    };

    struct Frustum::IMPL
    {
            IMPL() = default;
            ~IMPL() = default;

            template <Planes a, Planes b, Planes c>
            vec3 intersection(const vec3* crosses) const;

            vec4 planes[Count];
            vec3 points[8];
    };

    Frustum::Frustum() = default;
    Frustum::~Frustum() = default;

    Frustum::Frustum(const Frustum& other) : impl(new IMPL(*other.impl)) {}

    Frustum& Frustum::operator=(const Frustum& other)
    {
        if (this != &other)
        {
            impl = create_unique<IMPL>(*other.impl);
        }
        return *this;
    }

    Frustum::Frustum(mat4 m) : impl(new IMPL())
    {
        m = transpose(m);
        impl->planes[Left] = m[3] + m[0];
        impl->planes[Right] = m[3] - m[0];
        impl->planes[Bottom] = m[3] + m[1];
        impl->planes[Top] = m[3] - m[1];
        impl->planes[Near] = m[3] + m[2];
        impl->planes[Far] = m[3] - m[2];

        vec3 crosses[Combinations] = {cross(vec3(impl->planes[Left]), vec3(impl->planes[Right])),
                                      cross(vec3(impl->planes[Left]), vec3(impl->planes[Bottom])),
                                      cross(vec3(impl->planes[Left]), vec3(impl->planes[Top])),
                                      cross(vec3(impl->planes[Left]), vec3(impl->planes[Near])),
                                      cross(vec3(impl->planes[Left]), vec3(impl->planes[Far])),
                                      cross(vec3(impl->planes[Right]), vec3(impl->planes[Bottom])),
                                      cross(vec3(impl->planes[Right]), vec3(impl->planes[Top])),
                                      cross(vec3(impl->planes[Right]), vec3(impl->planes[Near])),
                                      cross(vec3(impl->planes[Right]), vec3(impl->planes[Far])),
                                      cross(vec3(impl->planes[Bottom]), vec3(impl->planes[Top])),
                                      cross(vec3(impl->planes[Bottom]), vec3(impl->planes[Near])),
                                      cross(vec3(impl->planes[Bottom]), vec3(impl->planes[Far])),
                                      cross(vec3(impl->planes[Top]), vec3(impl->planes[Near])),
                                      cross(vec3(impl->planes[Top]), vec3(impl->planes[Far])),
                                      cross(vec3(impl->planes[Near]), vec3(impl->planes[Far]))};

        impl->points[0] = impl->intersection<Left, Bottom, Near>(crosses);
        impl->points[1] = impl->intersection<Left, Top, Near>(crosses);
        impl->points[2] = impl->intersection<Right, Bottom, Near>(crosses);
        impl->points[3] = impl->intersection<Right, Top, Near>(crosses);
        impl->points[4] = impl->intersection<Left, Bottom, Far>(crosses);
        impl->points[5] = impl->intersection<Left, Top, Far>(crosses);
        impl->points[6] = impl->intersection<Right, Bottom, Far>(crosses);
        impl->points[7] = impl->intersection<Right, Top, Far>(crosses);
    }

    b8 Frustum::is_aabb_visible(const BoundingBox& aabb) const
    {
        const vec3& minp = aabb.min;
        const vec3& maxp = aabb.max;

        // check box outside/inside of frustum
        for (u32 i = 0; i < Count; i++)
        {
            if ((dot(impl->planes[i], vec4(minp.x, minp.y, minp.z, 1.0f)) < 0.0) &&
                (dot(impl->planes[i], vec4(maxp.x, minp.y, minp.z, 1.0f)) < 0.0) &&
                (dot(impl->planes[i], vec4(minp.x, maxp.y, minp.z, 1.0f)) < 0.0) &&
                (dot(impl->planes[i], vec4(maxp.x, maxp.y, minp.z, 1.0f)) < 0.0) &&
                (dot(impl->planes[i], vec4(minp.x, minp.y, maxp.z, 1.0f)) < 0.0) &&
                (dot(impl->planes[i], vec4(maxp.x, minp.y, maxp.z, 1.0f)) < 0.0) &&
                (dot(impl->planes[i], vec4(minp.x, maxp.y, maxp.z, 1.0f)) < 0.0) &&
                (dot(impl->planes[i], vec4(maxp.x, maxp.y, maxp.z, 1.0f)) < 0.0))
            {
                return false;
            }
        }

        // check frustum outside/inside box
        u32 out;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((impl->points[i].x > maxp.x) ? 1 : 0);
        if (out == 8) return false;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((impl->points[i].x < minp.x) ? 1 : 0);
        if (out == 8) return false;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((impl->points[i].y > maxp.y) ? 1 : 0);
        if (out == 8) return false;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((impl->points[i].y < minp.y) ? 1 : 0);
        if (out == 8) return false;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((impl->points[i].z > maxp.z) ? 1 : 0);
        if (out == 8) return false;

        out = 0;
        for (u32 i = 0; i < 8; i++) out += ((impl->points[i].z < minp.z) ? 1 : 0);
        if (out == 8) return false;

        return true;
    }

    template <Planes a, Planes b, Planes c>
    vec3 Frustum::IMPL::intersection(const vec3* crosses) const
    {
        const f32 D = dot(vec3(planes[a]), crosses[ij2k<b, c>::k]);
        const vec3 res = mat3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) *
                         vec3(planes[a].w, planes[b].w, planes[c].w);

        return res * (-1.0f / D);
    }

    std::vector<vec3> Frustum::get_points() const { return std::vector<vec3>(&impl->points[0], &impl->points[8]); }
};  // namespace mag
