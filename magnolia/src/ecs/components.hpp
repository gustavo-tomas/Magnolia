#pragma once

#include "core/types.hpp"

namespace mag
{
    using namespace mag::math;

    struct Component
    {
            virtual ~Component() = default;
    };

    struct NameComponent : public Component
    {
            NameComponent(const str& name) : name(name){};

            str name;
    };

    struct TransformComponent : public Component
    {
            TransformComponent(const vec3& translation = vec3(0), const vec3& rotation = vec3(0),
                               const vec3& scale = vec3(1))
                : translation(translation), rotation(rotation), scale(scale){};

            static mat4 get_transformation_matrix(const TransformComponent& transform);

            vec3 translation, rotation, scale;
    };

    // @TODO: i didnt turn Model into a component because then the ModelLoader would be loading components directly
    // and i find that a bit weird
    struct Model;
    struct ModelComponent : public Component
    {
            ModelComponent(const Model& model) : model(model) {}

            const Model& model;
    };

    struct LightComponent : public Component
    {
            LightComponent(const vec3& color = vec3(1), const f32 intensity = 1) : color(color), intensity(intensity) {}

            // This should match the shaders max number of lights
            static const u32 MAX_NUMBER_OF_LIGHTS = 4;

            vec3 color;
            f32 intensity;
    };

    inline mat4 TransformComponent::get_transformation_matrix(const TransformComponent& transform)
    {
        const quat pitch = angleAxis(radians(transform.rotation.x), vec3(1.0f, 0.0f, 0.0f));
        const quat yaw = angleAxis(radians(transform.rotation.y), vec3(0.0f, 1.0f, 0.0f));
        const quat roll = angleAxis(radians(transform.rotation.z), vec3(0.0f, 0.0f, 1.0f));

        const mat4 rotation_matrix = toMat4(roll) * toMat4(yaw) * toMat4(pitch);
        const mat4 translation_matrix = translate(mat4(1.0f), transform.translation);
        const mat4 scale_matrix = math::scale(mat4(1.0f), transform.scale);

        const mat4 model_matrix = translation_matrix * rotation_matrix * scale_matrix;

        return model_matrix;
    }
};  // namespace mag
