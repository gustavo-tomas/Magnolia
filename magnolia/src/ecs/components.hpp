#pragma once

#include <functional>

#include "camera/camera.hpp"
#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    using namespace mag::math;

    // @NOTE: beware of pointers! Deep copy also copies them over!

#define CLONE_DECLARATION(type) virtual Component* clone() const override;

    struct MAG_API Component
    {
            virtual ~Component();
            virtual Component* clone() const = 0;
    };

    struct MAG_API NameComponent : public Component
    {
            NameComponent(const str& name);

            CLONE_DECLARATION(NameComponent);

            str name;
    };

    struct MAG_API TransformComponent : public Component
    {
            TransformComponent(const vec3& translation = vec3(0), const vec3& rotation = vec3(0),
                               const vec3& scale = vec3(1));

            CLONE_DECLARATION(TransformComponent);

            vec3 translation, rotation, scale;

            mat4 get_transformation_matrix() const;
    };

    struct Image;
    struct MAG_API SpriteComponent : public Component
    {
            SpriteComponent(const ref<Image>& texture, const str& texture_file_path, const b8 constant_size = false,
                            const b8 always_face_camera = false);

            CLONE_DECLARATION(SpriteComponent);

            ref<Image> texture;
            str texture_file_path;  // @TODO: this is not ideal
            b8 constant_size;
            b8 always_face_camera;
    };

    // @NOTE: i didnt turn Model into a component because then the ModelLoader would be loading components directly
    // and i find that a bit weird
    struct Model;
    struct MAG_API ModelComponent : public Component
    {
            ModelComponent(const ref<Model>& model);

            CLONE_DECLARATION(ModelComponent);

            ref<Model> model;
    };

    struct Font;
    struct MAG_API TextComponent : public Component
    {
            TextComponent(const ref<Font>& font, const vec4& color, const str& text);

            CLONE_DECLARATION(TextComponent);

            ref<Font> font;
            vec4 color;
            str text;
    };

    struct Audio;
    struct MAG_API AudioComponent : public Component
    {
            AudioComponent(const ref<Audio>& audio, const f32 volume = 1.0f, const b8 play_on_load = false,
                           const vec3& position = vec3(0), const vec3& velocity = vec3(0));

            CLONE_DECLARATION(AudioComponent);

            ref<Audio> audio;
            f32 volume;
            vec3 position;
            vec3 velocity;
            b8 play_on_load;
    };

    struct MAG_API BoxColliderComponent : public Component
    {
            BoxColliderComponent(const vec3& dimensions = vec3(1));

            CLONE_DECLARATION(BoxColliderComponent);

            vec3 dimensions;
    };

    struct MAG_API RigidBodyComponent : public Component
    {
            RigidBodyComponent(const f32 mass = 0.0f);

            CLONE_DECLARATION(RigidBodyComponent);

            f32 mass;

            // Storage for physics engine use
            void* collision_object = nullptr;
    };

    struct MAG_API LightComponent : public Component
    {
            LightComponent(const vec3& color = vec3(1), const f32 intensity = 1);

            CLONE_DECLARATION(LightComponent);

            vec3 color;
            f32 intensity;
    };

    class Camera;
    struct MAG_API CameraComponent : public Component
    {
            CameraComponent(const Camera& camera);

            CLONE_DECLARATION(CameraComponent);

            Camera camera;
    };

    class ScriptableEntity;
    typedef std::function<ScriptableEntity*()> CreateScriptFn;
    typedef std::function<void(ScriptableEntity*)> DestroyScriptFn;

    struct MAG_API ScriptComponent : public Component
    {
            ScriptComponent(const str& file_path, void* handle = nullptr, CreateScriptFn create_entity = nullptr,
                            DestroyScriptFn destroy_entity = nullptr);

            CLONE_DECLARATION(ScriptComponent);

            CreateScriptFn create_entity;
            DestroyScriptFn destroy_entity;

            str file_path;
            void* handle = nullptr;
            ScriptableEntity* entity = nullptr;
    };
};  // namespace mag
