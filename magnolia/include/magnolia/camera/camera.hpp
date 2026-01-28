#pragma once

#include "magnolia/camera/frustum.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    using namespace mag::math;

    class MAG_API Camera
    {
        public:
            Camera() = default;
            virtual ~Camera() = default;

            void set_position(const vec3& position);
            void set_rotation(const vec3& rotation);
            void set_near_far(const vec2& near_far);
            void set_viewport_size(const vec2& size);

            const mat4& get_view() const;
            const mat4& get_projection() const;
            const vec3& get_position() const;
            const vec3& get_rotation() const;
            const mat4& get_rotation_mat() const;
            const Frustum& get_frustum() const;

            f32 get_near() const;
            f32 get_far() const;
            f32 get_aspect_ratio() const;

            vec3 get_side() const;
            vec3 get_up() const;
            vec3 get_forward() const;
            vec2 get_near_far() const;

            b8 is_aabb_visible(const BoundingBox& aabb) const;

        protected:
            virtual void calculate_projection() = 0;
            void calculate_frustum();

            mat4 projection = mat4(1.0f);
            f32 aspect_ratio = 16.0f / 9.0f;
            f32 near = 1.0f;
            f32 far = 100.0f;

        private:
            void calculate_view();

            Frustum frustum;
            mat4 view = mat4(1.0f);
            mat4 rotation_mat = mat4(1.0f);
            vec3 position = vec3(0.0f);
            vec3 rotation = vec3(0.0f);
    };

    struct PerspectiveCameraDesc
    {
            vec3 position = vec3(0.0f);
            vec3 rotation = vec3(0.0f);
            vec2 viewport_size = vec2(1280.0f, 720.0f);
            f32 near = 1.0f;
            f32 far = 1000.0f;
            f32 fov = 1.047198f;  // 60°
    };

    class MAG_API PerspectiveCamera : public Camera
    {
        public:
            PerspectiveCamera(const PerspectiveCameraDesc& camera_desc);
            ~PerspectiveCamera() override;

            void set_fov(const f32 fov);
            const f32& get_fov() const;

        protected:
            void calculate_projection() override;

        private:
            f32 fov;
    };

    struct OrthographicCameraDesc
    {
            vec3 position = vec3(0.0f);
            vec3 rotation = vec3(0.0f);
            vec2 viewport_size = vec2(1280.0f, 720.0f);
            f32 near = -1.0f;
            f32 far = 1000.0f;
            f32 size = 1000.0f;
    };

    class MAG_API OrthographicCamera : public Camera
    {
        public:
            OrthographicCamera(const OrthographicCameraDesc& camera_desc);
            ~OrthographicCamera() override;

            void set_size(const f32 size);
            f32 get_size() const;

        protected:
            void calculate_projection() override;

        private:
            f32 size;
    };
};  // namespace mag
