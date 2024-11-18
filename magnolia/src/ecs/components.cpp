#include "ecs/components.hpp"

#include "math/generic.hpp"
#include "renderer/test_model.hpp"

namespace mag
{
#define CLONE_DEFINITION(type) \
    Component* type::clone() const { return new type(*this); }

    Component::~Component() = default;

    NameComponent::NameComponent(const str& name) : name(name) {}

    TransformComponent::TransformComponent(const vec3& translation, const vec3& rotation, const vec3& scale)
        : translation(translation), rotation(rotation), scale(scale)
    {
    }

    mat4 TransformComponent::get_transformation_matrix() const
    {
        const mat4 rotation_mat = math::toMat4(quat(math::radians(rotation)));

        return translate(mat4(1.0f), translation) * rotation_mat * math::scale(mat4(1.0f), scale);
    }

    SpriteComponent::SpriteComponent(const ref<Image>& texture, const str& texture_file_path, const b8 constant_size,
                                     const b8 always_face_camera)
        : texture(texture),
          texture_file_path(texture_file_path),
          constant_size(constant_size),
          always_face_camera(always_face_camera)
    {
    }

    ModelComponent::ModelComponent(const ref<Model>& model) : model(model) {}

    BoxColliderComponent::BoxColliderComponent(const vec3& dimensions) : dimensions(dimensions) {}

    RigidBodyComponent::RigidBodyComponent(const f32 mass) : mass(mass) {}

    b8 RigidBodyComponent::is_dynamic() const { return mass != 0.0f; }

    LightComponent::LightComponent(const vec3& color, const f32 intensity) : color(color), intensity(intensity) {}

    CameraComponent::CameraComponent(const Camera& camera) : camera(camera) {}

    ScriptComponent::ScriptComponent(const str& file_path, void* handle, CreateScriptFn create_entity,
                                     DestroyScriptFn destroy_entity)
        : create_entity(create_entity), destroy_entity(destroy_entity), file_path(file_path), handle(handle)
    {
    }

    SkydomeComponent::SkydomeComponent(Skydome* skydome) : skydome(skydome) {}

    SkydomeComponent::~SkydomeComponent() { delete skydome; }

    CLONE_DEFINITION(NameComponent);
    CLONE_DEFINITION(TransformComponent);
    CLONE_DEFINITION(SpriteComponent);
    CLONE_DEFINITION(ModelComponent);
    CLONE_DEFINITION(BoxColliderComponent);
    CLONE_DEFINITION(RigidBodyComponent);
    CLONE_DEFINITION(LightComponent);
    CLONE_DEFINITION(CameraComponent);
    CLONE_DEFINITION(ScriptComponent);
    CLONE_DEFINITION(SkydomeComponent);
};  // namespace mag
