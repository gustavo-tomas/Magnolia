#pragma once

#include <magnolia/camera/camera.hpp>
#include <magnolia/core/types.hpp>
#include <magnolia/math/types.hpp>

namespace mag
{
    struct FontResource;
    class PerspectiveCamera;
};  // namespace mag

namespace game
{
    using namespace mag::math;

    struct TransformComponent
    {
            TransformComponent(const vec3& translation = vec3(0), const quat& rotation = quat(),
                               const vec3& scale = vec3(1))
                : rotation(rotation), translation(translation), scale(scale)
            {
            }

            mat4 get_transformation_matrix() const
            {
                const mat4 rotation_mat = mag::math::toMat4(rotation);

                return translate(mat4(1.0f), translation) * rotation_mat * mag::math::scale(mat4(1.0f), scale);
            }

            quat rotation;
            vec3 translation;
            vec3 scale;
    };

    struct TextComponent
    {
            TextComponent(const mag::ref<mag::FontResource>& font, const vec4& color, const str& text)
                : font(font), color(color), text(text)
            {
            }

            mag::ref<mag::FontResource> font;
            vec4 color;
            str text;
    };

    struct CameraComponent
    {
            CameraComponent(const mag::PerspectiveCamera& camera) : camera(camera) {}

            mag::PerspectiveCamera camera;
    };
};  // namespace game
