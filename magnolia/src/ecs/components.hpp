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

    struct BoxColliderComponent : public Component
    {
            BoxColliderComponent(const vec3& dimensions = vec3(1)) : dimensions(dimensions) {}

            CLONE(BoxColliderComponent);

            vec3 dimensions;

            // Storage for physics engine use
            void* internal;
    };

    struct RigidBodyComponent : public Component
    {
            RigidBodyComponent(const f32 mass = 0.0f) : mass(mass) {}

            CLONE(RigidBodyComponent);

            f32 mass;

            // Storage for physics engine use
            void* internal;

            b8 is_dynamic() const { return mass != 0.0f; }
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

    class ScriptableEntity;
    struct NativeScriptComponent : public Component
    {
            CLONE(NativeScriptComponent);

            ScriptableEntity* instance = nullptr;

            ScriptableEntity* (*instanciate_script)();
            void (*destroy_script)(NativeScriptComponent*);

            template <typename T>
            void bind()
            {
                static_assert(std::is_base_of<ScriptableEntity, T>::value, "T must be derived from ScriptableEntity");

                instanciate_script = []() { return static_cast<ScriptableEntity*>(new T()); };
                destroy_script = [](NativeScriptComponent* nsc)
                {
                    delete nsc->instance;
                    nsc->instance = nullptr;
                };
            }
    };
};  // namespace mag
