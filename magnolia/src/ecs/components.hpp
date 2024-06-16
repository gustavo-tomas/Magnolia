#pragma once

#include <memory>

#include "core/types.hpp"

namespace mag
{
    using namespace mag::math;

#define CLONE(type) \
    std::unique_ptr<Component> clone() const override { return std::make_unique<type>(*this); }

    struct Component
    {
            virtual ~Component() = default;
            virtual std::unique_ptr<Component> clone() const = 0;
    };

    struct NameComponent : public Component
    {
            NameComponent(const str& name) : name(name){};

            CLONE(NameComponent);

            str name;
    };

    struct TransformComponent : public Component
    {
            TransformComponent(const vec3& translation = vec3(0), const vec3& rotation = vec3(0),
                               const vec3& scale = vec3(1))
                : translation(translation), rotation(rotation), scale(scale){};

            CLONE(TransformComponent);

            vec3 translation, rotation, scale;

            mat4 get_transformation_matrix() const
            {
                const mat4 rotation_mat = math::toMat4(quat(math::radians(rotation)));

                return translate(mat4(1.0f), translation) * rotation_mat * math::scale(mat4(1.0f), scale);
            }
    };

    // @TODO: i didnt turn Model into a component because then the ModelLoader would be loading components directly
    // and i find that a bit weird
    struct Model;
    struct ModelComponent : public Component
    {
            ModelComponent(const Model& model) : model(model) {}

            CLONE(ModelComponent);

            const Model& model;

            u32 albedo_descriptor_offset;  // @TODO: temporary fix for descriptor chicanery
            u32 normal_descriptor_offset;  // @TODO: temporary fix for descriptor chicanery
    };

    // Basically a wrapper for bullet physics
    struct BulletCollisionShape;
    struct CollisionShapeComponent : public Component
    {
            CollisionShapeComponent(BulletCollisionShape* shape) : shape(shape) {}

            CLONE(CollisionShapeComponent);

            BulletCollisionShape* shape;
    };

    // Basically a wrapper for bullet physics
    struct BulletRigidBody;
    struct RigidBodyComponent : public Component
    {
            RigidBodyComponent(BulletRigidBody* rigid_body) : rigid_body(rigid_body) {}

            CLONE(RigidBodyComponent);

            BulletRigidBody* rigid_body;
    };

    struct LightComponent : public Component
    {
            LightComponent(const vec3& color = vec3(1), const f32 intensity = 1) : color(color), intensity(intensity) {}

            CLONE(LightComponent);

            // This should match the shaders max number of lights
            static const u32 MAX_NUMBER_OF_LIGHTS = 4;

            vec3 color;
            f32 intensity;
    };
};  // namespace mag
