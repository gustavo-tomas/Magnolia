#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    class MAG_API Camera
    {
        public:
            Camera() = default;
            virtual ~Camera() = default;

            void set_position(const math::vec3& camera_position);
            void set_rotation(const math::quat& rotation);
            void set_near_far(const math::vec2& near_far);
            void set_viewport_size(const math::vec2& size);

            const math::mat4& get_view() const;
            const math::mat4& get_projection() const;
            const math::vec3& get_position() const;
            math::quat get_rotation() const;
            const math::mat4& get_rotation_mat() const;

            f32 get_near() const;
            f32 get_far() const;
            f32 get_aspect_ratio() const;

            math::vec3 get_side() const;
            math::vec3 get_up() const;
            math::vec3 get_forward() const;
            math::vec2 get_near_far() const;

        protected:
            virtual void calculate_projection() = 0;

            math::mat4 projection = math::mat4(1.0F);
            f32 aspect_ratio = 16.0F / 9.0F;
            f32 near = 1.0F;
            f32 far = 100.0F;

        private:
            void calculate_view();

            math::mat4 view = math::mat4(1.0F);
            math::mat4 rotation_mat = math::mat4(1.0F);
            math::vec3 position = math::vec3(0.0F);
    };

    struct PerspectiveCameraDesc
    {
            math::vec3 position = math::vec3(0.0F);
            math::quat rotation = math::quat(1.0F, 0.0F, 0.0F, 0.0F);
            math::vec2 viewport_size = math::vec2(1280.0F, 720.0F);
            f32 near = 1.0F;
            f32 far = 1000.0F;
            f32 fov = 1.047198F;  // 60°
    };

    class MAG_API PerspectiveCamera : public Camera
    {
        public:
            explicit PerspectiveCamera(const PerspectiveCameraDesc& camera_desc);
            ~PerspectiveCamera() override;

            void set_fov(f32 camera_fov);
            const f32& get_fov() const;

        protected:
            void calculate_projection() override;

        private:
            f32 fov = 1.047198F;  // 60°;
    };

    struct OrthographicCameraDesc
    {
            math::vec3 position = math::vec3(0.0F);
            math::quat rotation = math::quat(1.0F, 0.0F, 0.0F, 0.0F);
            math::vec2 viewport_size = math::vec2(1280.0F, 720.0F);
            f32 near = -1.0F;
            f32 far = 1000.0F;
            f32 size = 1000.0F;
    };

    class MAG_API OrthographicCamera : public Camera
    {
        public:
            explicit OrthographicCamera(const OrthographicCameraDesc& camera_desc);
            ~OrthographicCamera() override;

            void set_size(f32 camera_size);
            f32 get_size() const;

        protected:
            void calculate_projection() override;

        private:
            f32 size = 1000.0F;
    };
};  // namespace mag
