#pragma once

#include <functional>

#include "magnolia/camera/camera.hpp"
#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    using namespace mag::math;

    struct MAG_API NameComponent
    {
            NameComponent(const str& name);

            str name;
    };

    struct MAG_API TransformComponent
    {
            TransformComponent(const vec3& translation = vec3(0), const vec3& rotation = vec3(0),
                               const vec3& scale = vec3(1));

            vec3 translation, rotation, scale;

            mat4 get_transformation_matrix() const;
    };

    struct TextureResource;
    struct MAG_API SpriteComponent
    {
            SpriteComponent(const ref<TextureResource>& texture, const b8 constant_size = false,
                            const b8 always_face_camera = false);

            ref<TextureResource> texture;
            b8 constant_size;
            b8 always_face_camera;
    };

    struct ModelResource;
    struct MAG_API ModelComponent
    {
            ModelComponent(const ref<ModelResource>& model);

            ref<ModelResource> model;
    };

    struct FontResource;
    struct MAG_API TextComponent
    {
            TextComponent(const ref<FontResource>& font, const vec4& color, const str& text);

            ref<FontResource> font;
            vec4 color;
            str text;
    };

    struct AudioResource;
    struct MAG_API AudioComponent
    {
            AudioComponent(const ref<AudioResource>& audio, const f32 volume = 1.0f, const b8 play_on_load = false,
                           const vec3& position = vec3(0), const vec3& velocity = vec3(0));

            ref<AudioResource> audio;
            f32 volume;
            vec3 position;
            vec3 velocity;
            b8 play_on_load;
    };

    struct MAG_API ColliderComponent
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

            ColliderComponent(const ColliderType collider_type, const Collider collider);
    };

    struct MAG_API RigidBodyComponent
    {
            RigidBodyComponent(const f32 mass = 0.0f);

            f32 mass;

            // Storage for physics engine use
            void* collision_object = nullptr;
    };

    struct MAG_API LightComponent
    {
            LightComponent(const vec3& color = vec3(1), const f32 intensity = 1);

            vec3 color;
            f32 intensity;
    };

    class PerspectiveCamera;
    struct MAG_API CameraComponent
    {
            CameraComponent(const PerspectiveCamera& camera);

            PerspectiveCamera camera;
    };

    class ScriptableEntity;
    typedef std::function<ScriptableEntity*()> CreateScriptFn;
    typedef std::function<void(ScriptableEntity*)> DestroyScriptFn;

    struct MAG_API ScriptComponent
    {
            ScriptComponent(const str& file_path, void* handle = nullptr, CreateScriptFn create_entity = nullptr,
                            DestroyScriptFn destroy_entity = nullptr);

            CreateScriptFn create_entity;
            DestroyScriptFn destroy_entity;

            str file_path;
            void* handle = nullptr;
            ScriptableEntity* entity = nullptr;
    };
};  // namespace mag
