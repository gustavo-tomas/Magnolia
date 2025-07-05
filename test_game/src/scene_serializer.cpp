#include "scene_serializer.hpp"

#include <magnolia/camera/camera.hpp>
#include <magnolia/ecs/components.hpp>
#include <magnolia/ecs/ecs.hpp>
#include <magnolia/platform/file_system.hpp>
#include <magnolia/platform/serializer.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/resources/audio.hpp>
#include <magnolia/resources/font.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/texture.hpp>
#include <magnolia/scene/scene.hpp>

namespace game
{
    mag::fs::json& operator<<(mag::fs::json& out, const mag::vec2& v)
    {
        for (i32 i = 0; i < v.length(); i++) out.push_back(v[i]);
        return out;
    }

    mag::fs::json& operator<<(mag::fs::json& out, const mag::vec3& v)
    {
        for (i32 i = 0; i < v.length(); i++) out.push_back(v[i]);
        return out;
    }

    mag::fs::json& operator<<(mag::fs::json& out, const mag::vec4& v)
    {
        for (i32 i = 0; i < v.length(); i++) out.push_back(v[i]);
        return out;
    }

    namespace scene
    {
        b8 save(const str& file_path, mag::Scene& scene)
        {
            mag::fs::Serializer serializer;

            serializer.register_on_save_handler<mag::Scene>(
                [](mag::Scene& scene, mag::fs::Serializer& s)
                {
                    mag::fs::json& data = s.json;

                    // Serialize scene data to file
                    data["Type"] = "Scene";
                    data["Name"] = scene.get_name();

                    auto& ecs = scene.get_ecs();
                    for (const auto entity_id : ecs.get_entities_ids())
                    {
                        mag::fs::json entity;

                        if (auto component = ecs.get_component<mag::NameComponent>(entity_id))
                        {
                            entity["NameComponent"]["Name"] = component->name;
                        }

                        if (auto component = ecs.get_component<mag::TransformComponent>(entity_id))
                        {
                            entity["TransformComponent"]["Translation"] << component->translation;
                            entity["TransformComponent"]["Rotation"] << component->rotation;
                            entity["TransformComponent"]["Scale"] << component->scale;
                        }

                        if (auto component = ecs.get_component<mag::ModelComponent>(entity_id))
                        {
                            if (component->model->file_path.empty())
                            {
                                LOG_WARNING("Model {0} has no file path and will not be serialized",
                                            component->model->name);
                            }

                            else
                            {
                                entity["ModelComponent"]["Name"] = component->model->name;
                                entity["ModelComponent"]["FilePath"] = component->model->file_path;
                            }
                        }

                        if (auto component = ecs.get_component<mag::SpriteComponent>(entity_id))
                        {
                            if (component->texture->file_path.empty())
                            {
                                LOG_WARNING("Sprite has no file path and will not be serialized");
                            }

                            else
                            {
                                entity["SpriteComponent"]["FilePath"] = component->texture->file_path;
                                entity["SpriteComponent"]["ConstantSize"] = component->constant_size;
                                entity["SpriteComponent"]["AlwaysFaceCamera"] = component->always_face_camera;
                            }
                        }

                        if (auto component = ecs.get_component<mag::TextComponent>(entity_id))
                        {
                            entity["TextComponent"]["FilePath"] = component->font->file_path;
                            entity["TextComponent"]["Text"] = component->text;
                            entity["TextComponent"]["Color"] << component->color;
                        }

                        if (auto component = ecs.get_component<mag::AudioComponent>(entity_id))
                        {
                            entity["AudioComponent"]["FilePath"] = component->audio->file_path;
                            entity["AudioComponent"]["Volume"] = component->volume;
                            entity["AudioComponent"]["PlayOnLoad"] = component->play_on_load;
                            entity["AudioComponent"]["Position"] << component->position;
                            entity["AudioComponent"]["Velocity"] << component->velocity;
                        }

                        if (auto component = ecs.get_component<mag::ColliderComponent>(entity_id))
                        {
                            switch (component->collider_type)
                            {
                                case mag::ColliderComponent::ColliderType::Box:
                                    entity["BoxColliderComponent"]["Dimensions"] << component->collider.box.dimensions;
                                    break;

                                case mag::ColliderComponent::ColliderType::Capsule:
                                    entity["CapsuleColliderComponent"]["Radius"] = component->collider.capsule.radius;
                                    entity["CapsuleColliderComponent"]["Height"] = component->collider.capsule.height;
                                    break;

                                default:
                                    MAG_ASSERT(false, "Unhandled collider type");
                                    break;
                            }
                        }

                        if (auto component = ecs.get_component<mag::RigidBodyComponent>(entity_id))
                        {
                            entity["RigidBodyComponent"]["Mass"] = component->mass;
                        }

                        if (auto component = ecs.get_component<mag::LightComponent>(entity_id))
                        {
                            entity["LightComponent"]["Color"] << component->color;
                            entity["LightComponent"]["Intensity"] = component->intensity;
                        }

                        if (auto component = ecs.get_component<mag::CameraComponent>(entity_id))
                        {
                            entity["CameraComponent"]["Fov"] = component->camera.get_fov();
                            entity["CameraComponent"]["Near"] = component->camera.get_near();
                            entity["CameraComponent"]["Far"] = component->camera.get_far();
                        }

                        if (auto component = ecs.get_component<mag::ScriptComponent>(entity_id))
                        {
                            entity["ScriptComponent"]["FilePath"] = component->file_path;
                        }

                        data["Entities"].push_back(entity);
                    }
                });

            if (!serializer.save(file_path, scene))
            {
                LOG_ERROR("Failed to save scene to file: '{0}'", file_path);
                return false;
            }

            return true;
        }

        b8 load(const str& file_path, mag::Scene& scene)
        {
            mag::fs::Serializer serializer;

            serializer.register_on_load_handler<mag::Scene>(
                [](mag::Scene& scene, mag::fs::Serializer& s)
                {
                    mag::fs::json& data = s.json;

                    const str scene_name = data["Name"];
                    scene.set_name(scene_name);

                    LOG_INFO("Deserializing scene '{0}'", scene_name);

                    if (!data.contains("Entities"))
                    {
                        return;
                    }

                    auto& ecs = scene.get_ecs();

                    for (auto& entity : data["Entities"])
                    {
                        const u32 entity_id = ecs.create_entity();

                        if (entity.contains("NameComponent"))
                        {
                            const str entity_name = entity["NameComponent"]["Name"];
                            ecs.get_component<mag::NameComponent>(entity_id)->name = entity_name;
                        }

                        if (entity.contains("TransformComponent"))
                        {
                            const auto& component = entity["TransformComponent"];

                            mag::vec3 translation = mag::vec3(0);
                            mag::vec3 rotation = mag::vec3(0);
                            mag::vec3 scale = mag::vec3(0);

                            // @TODO: dry this
                            for (i32 i = 0; i < translation.length(); i++)
                                translation[i] = component["Translation"][i].get<f32>();

                            for (i32 i = 0; i < rotation.length(); i++)
                                rotation[i] = component["Rotation"][i].get<f32>();

                            for (i32 i = 0; i < scale.length(); i++) scale[i] = component["Scale"][i].get<f32>();

                            ecs.add_component(entity_id, new mag::TransformComponent(translation, rotation, scale));
                        }

                        if (entity.contains("ModelComponent"))
                        {
                            const auto& component = entity["ModelComponent"];
                            const str file_path = component["FilePath"];

                            const auto& model = mag::resource::get_model(file_path);

                            ecs.add_component(entity_id, new mag::ModelComponent(model));
                        }

                        if (entity.contains("SpriteComponent"))
                        {
                            const auto& component = entity["SpriteComponent"];
                            const str file_path = component["FilePath"];
                            const b8 constant_size = component["ConstantSize"].get<b8>();
                            const b8 always_face_camera = component["AlwaysFaceCamera"].get<b8>();

                            const auto& sprite = mag::resource::get_texture(file_path);

                            ecs.add_component(entity_id,
                                              new mag::SpriteComponent(sprite, constant_size, always_face_camera));
                        }

                        if (entity.contains("TextComponent"))
                        {
                            const auto& component = entity["TextComponent"];
                            const str file_path = component["FilePath"];
                            const str text = component["Text"];
                            mag::vec4 color = mag::vec4(0.0f);

                            for (i32 i = 0; i < color.length(); i++) color[i] = component["Color"][i].get<f32>();

                            const auto& font = mag::resource::get_font(file_path);

                            ecs.add_component(entity_id, new mag::TextComponent(font, color, text));
                        }

                        if (entity.contains("AudioComponent"))
                        {
                            const auto& component = entity["AudioComponent"];
                            const str file_path = component["FilePath"];

                            const f32 volume = component["Volume"].get<f32>();
                            const b8 play_on_load = component["PlayOnLoad"].get<f32>();
                            mag::vec3 position = mag::vec3(0.0f);
                            mag::vec3 velocity = mag::vec3(0.0f);

                            for (i32 i = 0; i < position.length(); i++)
                                position[i] = component["Position"][i].get<f32>();
                            for (i32 i = 0; i < velocity.length(); i++)
                                velocity[i] = component["Velocity"][i].get<f32>();

                            const auto& audio = mag::resource::get_audio(file_path);

                            ecs.add_component(entity_id,
                                              new mag::AudioComponent(audio, volume, play_on_load, position, velocity));
                        }

                        if (entity.contains("BoxColliderComponent"))
                        {
                            const auto& component = entity["BoxColliderComponent"];

                            mag::vec3 dimensions = mag::vec3(0);

                            for (i32 i = 0; i < dimensions.length(); i++)
                                dimensions[i] = component["Dimensions"][i].get<f32>();

                            mag::ColliderComponent::Collider collider = {};
                            collider.box.dimensions = dimensions;

                            ecs.add_component(entity_id, new mag::ColliderComponent(
                                                             mag::ColliderComponent::ColliderType::Box, collider));
                        }

                        if (entity.contains("CapsuleColliderComponent"))
                        {
                            const auto& component = entity["CapsuleColliderComponent"];

                            mag::ColliderComponent::Collider collider = {};
                            collider.capsule.radius = component["Radius"].get<f32>();
                            collider.capsule.height = component["Height"].get<f32>();

                            ecs.add_component(entity_id, new mag::ColliderComponent(
                                                             mag::ColliderComponent::ColliderType::Capsule, collider));
                        }

                        if (entity.contains("RigidBodyComponent"))
                        {
                            const auto& component = entity["RigidBodyComponent"];

                            f32 mass = component["Mass"].get<f32>();

                            ecs.add_component(entity_id, new mag::RigidBodyComponent(mass));
                        }

                        if (entity.contains("LightComponent"))
                        {
                            const auto& component = entity["LightComponent"];

                            mag::vec3 color = mag::vec3(0);
                            f32 intensity = 0;

                            for (i32 i = 0; i < color.length(); i++) color[i] = component["Color"][i].get<f32>();
                            intensity = component["Intensity"].get<f32>();

                            ecs.add_component(entity_id, new mag::LightComponent(color, intensity));
                        }

                        if (entity.contains("CameraComponent"))
                        {
                            const auto& component = entity["CameraComponent"];

                            const f32 fov = component["Fov"].get<f32>();
                            const f32 near = component["Near"].get<f32>();
                            const f32 far = component["Far"].get<f32>();

                            mag::PerspectiveCameraDesc camera_desc = {};
                            camera_desc.near = near;
                            camera_desc.far = far;
                            camera_desc.fov = fov;
                            camera_desc.viewport_size = mag::window::get_size();
                            camera_desc.position = mag::vec3(0.0f);
                            camera_desc.rotation = mag::vec3(0.0f);

                            mag::PerspectiveCamera camera = mag::PerspectiveCamera(camera_desc);

                            ecs.add_component(entity_id, new mag::CameraComponent(camera));
                        }

                        if (entity.contains("ScriptComponent"))
                        {
                            const auto& component = entity["ScriptComponent"];

                            const str file_path = component["FilePath"];

                            ecs.add_component(entity_id, new mag::ScriptComponent(file_path));
                        }
                    }
                });

            if (!serializer.load(file_path, scene))
            {
                LOG_ERROR("Failed to load scene to file: '{0}'", file_path);
                return false;
            }

            return true;
        }
    };  // namespace scene
};      // namespace game
