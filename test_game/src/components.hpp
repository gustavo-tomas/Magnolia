#pragma once

#include <functional>
#include <magnolia/camera/camera.hpp>
#include <magnolia/core/types.hpp>
#include <magnolia/math/types.hpp>

namespace mag
{
    struct TextureResource;
    struct FontResource;
    struct AudioResource;
    struct ModelResource;
    class PerspectiveCamera;
};  // namespace mag

namespace game
{
    using namespace mag::math;

    struct NameComponent
    {
            NameComponent(const str& name) : name(name) {}

            str name;
    };

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

    struct SpriteComponent
    {
            SpriteComponent(const mag::ref<mag::TextureResource>& texture, const b8 constant_size = false,
                            const b8 always_face_camera = false)
                : texture(texture), constant_size(constant_size), always_face_camera(always_face_camera)
            {
            }

            mag::ref<mag::TextureResource> texture;
            b8 constant_size;
            b8 always_face_camera;
    };

    struct ModelComponent
    {
            ModelComponent(const mag::ref<mag::ModelResource>& model) : model(model) {}

            mag::ref<mag::ModelResource> model;
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

    struct AudioComponent
    {
            AudioComponent(const mag::ref<mag::AudioResource>& audio, const f32 volume = 1.0f,
                           const b8 play_on_load = false, const vec3& position = vec3(0),
                           const vec3& velocity = vec3(0))
                : audio(audio), volume(volume), position(position), velocity(velocity), play_on_load(play_on_load)
            {
            }

            mag::ref<mag::AudioResource> audio;
            f32 volume;
            vec3 position;
            vec3 velocity;
            b8 play_on_load;
    };

    struct ColliderComponent
    {
            enum class ColliderType
            {
                Box,
                Capsule
            } collider_type;

            struct BoxCollider
            {
                    vec3 dimensions = vec3(1.0f);
            };

            struct CapsuleCollider
            {
                    f32 radius = 1.0f;
                    f32 height = 1.0f;
            };

            union Collider
            {
                    struct BoxCollider box = {};
                    struct CapsuleCollider capsule;
            } collider;

            ColliderComponent(const ColliderType collider_type, const Collider collider)
                : collider_type(collider_type), collider(collider)
            {
            }
    };

    struct RigidBodyComponent
    {
            RigidBodyComponent(const f32 mass) : mass(mass) {}

            f32 mass;

            // Storage for physics engine use
            void* collision_object = nullptr;
    };

    struct LightComponent
    {
            LightComponent(const vec3& color = vec3(1), const f32 intensity = 1) : color(color), intensity(intensity) {}

            vec3 color;
            f32 intensity;
    };

    struct CameraComponent
    {
            CameraComponent(const mag::PerspectiveCamera& camera) : camera(camera) {}

            mag::PerspectiveCamera camera;
    };

    class ScriptableEntity;
    typedef std::function<ScriptableEntity*()> CreateScriptFn;
    typedef std::function<void(ScriptableEntity*)> DestroyScriptFn;

    struct ScriptComponent
    {
            ScriptComponent(const str& file_path, void* handle = nullptr, CreateScriptFn create_entity = nullptr,
                            DestroyScriptFn destroy_entity = nullptr)
                : create_entity(create_entity), destroy_entity(destroy_entity), file_path(file_path), handle(handle)
            {
            }

            CreateScriptFn create_entity;
            DestroyScriptFn destroy_entity;

            str file_path;
            void* handle = nullptr;
            ScriptableEntity* entity = nullptr;
    };
};  // namespace game
