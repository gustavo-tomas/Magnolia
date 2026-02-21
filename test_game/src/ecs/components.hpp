#pragma once

#include <functional>
#include <magnolia/camera/camera.hpp>
#include <magnolia/core/types.hpp>
#include <magnolia/math/types.hpp>
#include <magnolia/physics/physics.hpp>
#include <magnolia/scripting/scripting_engine.hpp>
#include <utility>
#include <variant>

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

    struct BoxCollider
    {
            BoxCollider(const vec3& dimensions = vec3(1.0f)) : dimensions(dimensions) {}

            vec3 dimensions;
    };

    struct CapsuleCollider
    {
            CapsuleCollider(const f32 radius = 1.0f, const f32 height = 1.0f) : radius(radius), height(height) {}

            f32 radius;
            f32 height;
    };

    struct MeshCollider
    {
            MeshCollider(const str& file_path, const std::vector<mag::math::Triangle>& triangles)
                : file_path(file_path), triangles(triangles)
            {
            }

            str file_path;
            std::vector<mag::math::Triangle> triangles;
    };

    using Collider = std::variant<BoxCollider, CapsuleCollider, MeshCollider>;

    struct RigidBodyComponent
    {
            RigidBodyComponent(const Collider& collider, const f32 mass) : mass(mass), collider(collider) {}

            f32 mass;
            Collider collider;
            mag::RigidBodyHandle rigid_body_handle = {};
    };

    struct LightComponent
    {
            LightComponent(const vec3& color = vec3(1), const f32 intensity = 1) : color(color), intensity(intensity) {}

            vec3 color;
            f32 intensity;
    };

    struct PerspectiveCameraComponent
    {
            PerspectiveCameraComponent(const mag::PerspectiveCamera& camera) : camera(camera) {}

            mag::PerspectiveCamera camera;
    };

    struct OrthographicCameraComponent
    {
            OrthographicCameraComponent(const mag::OrthographicCamera& camera) : camera(camera) {}

            mag::OrthographicCamera camera;
    };

    class ScriptableEntity;
    using CreateScriptFn = std::function<ScriptableEntity*()>;
    using DestroyScriptFn = std::function<void(ScriptableEntity*)>;

    struct ScriptComponent
    {
            ScriptComponent(str file_path, const mag::script::ScriptHandle handle = mag::Invalid_ID,
                            CreateScriptFn create_entity = nullptr, DestroyScriptFn destroy_entity = nullptr)
                : create_entity(std::move(create_entity)),
                  destroy_entity(std::move(destroy_entity)),
                  file_path(std::move(file_path)),
                  handle(handle)
            {
            }

            CreateScriptFn create_entity;
            DestroyScriptFn destroy_entity;

            str file_path;
            mag::script::ScriptHandle handle = mag::Invalid_ID;
            ScriptableEntity* entity = nullptr;
    };

    struct PlayerComponent
    {
            vec3 camera_offset = vec3(50.0f);
            vec3 bullet_offset = vec3(50.0f);

            f32 hp = 100.0f;
            f32 walk_speed = 3000.0f;
            f32 mouse_sensitivity = 0.2f;
            f32 fire_rate = 20.0f;  // per second

            f32 pitch = 0.0f;
            f32 yaw = 0.0f;
    };

    struct BulletComponent
    {
            BulletComponent(const f32 time_to_live = 10.0f) : time_to_live(time_to_live) {}

            f32 time_to_live;  // seconds
    };

    // Debug
    struct DebugComponent
    {
            DebugComponent() = default;
    };
};  // namespace game
