#include "magnolia/ecs/components.hpp"

#include "magnolia/math/types.hpp"

namespace mag
{
    NameComponent::NameComponent(const str& name) : name(name) {}

    TransformComponent::TransformComponent(const vec3& translation, const vec3& rotation, const vec3& scale)
        : translation(translation), rotation(rotation), scale(scale)
    {
    }

    mat4 TransformComponent::get_transformation_matrix() const
    {
        const mat4 rotation_mat = math::toMat4(quat(rotation));

        return translate(mat4(1.0f), translation) * rotation_mat * math::scale(mat4(1.0f), scale);
    }

    SpriteComponent::SpriteComponent(const ref<TextureResource>& texture, const b8 constant_size,
                                     const b8 always_face_camera)
        : texture(texture), constant_size(constant_size), always_face_camera(always_face_camera)
    {
    }

    ModelComponent::ModelComponent(const ref<ModelResource>& model) : model(model) {}

    TextComponent::TextComponent(const ref<FontResource>& font, const vec4& color, const str& text)
        : font(font), color(color), text(text)
    {
    }

    AudioComponent::AudioComponent(const ref<AudioResource>& audio, const f32 volume, const b8 play_on_load,
                                   const vec3& position, const vec3& velocity)
        : audio(audio), volume(volume), position(position), velocity(velocity), play_on_load(play_on_load)
    {
    }

    ColliderComponent::ColliderComponent(const ColliderType collider_type, const Collider collider)
        : collider_type(collider_type), collider(collider)
    {
    }

    RigidBodyComponent::RigidBodyComponent(const f32 mass) : mass(mass) {}

    LightComponent::LightComponent(const vec3& color, const f32 intensity) : color(color), intensity(intensity) {}

    CameraComponent::CameraComponent(const PerspectiveCamera& camera) : camera(camera) {}

    ScriptComponent::ScriptComponent(const str& file_path, void* handle, CreateScriptFn create_entity,
                                     DestroyScriptFn destroy_entity)
        : create_entity(create_entity), destroy_entity(destroy_entity), file_path(file_path), handle(handle)
    {
    }
};  // namespace mag
