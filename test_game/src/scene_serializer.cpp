#include "scene_serializer.hpp"

#include <magnolia/camera/camera.hpp>
#include <magnolia/core/assert.hpp>
#include <magnolia/core/logger.hpp>
#include <magnolia/ecs/ecs.hpp>
#include <magnolia/physics/physics.hpp>
#include <magnolia/platform/file_system.hpp>
#include <magnolia/platform/json.hpp>
#include <magnolia/platform/serializer.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/resources/audio.hpp>
#include <magnolia/resources/font.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/resource.hpp>
#include <magnolia/resources/texture.hpp>

#include "ecs/components.hpp"
#include "scene.hpp"

namespace nlohmann
{
    // @TODO: error/data/format checking when everything is stable

    inline void from_json(const mag::fs::json& data, game::DebugComponent& component)
    {
        (void)data;
        (void)component;
    }

    inline void from_json(const mag::fs::json& data, game::NameComponent& component)
    {
        component.name = data["Name"].get<str>();
    }

    inline void from_json(const mag::fs::json& data, game::TransformComponent& component)
    {
        component.translation = data["Translation"].get<mag::math::vec3>();
        component.rotation = data["Rotation"].get<mag::math::quat>();
        component.scale = data["Scale"].get<mag::math::vec3>();
    }
    inline void from_json(const mag::fs::json& data, game::LightComponent& component)
    {
        component.color = data["Color"].get<mag::math::vec3>();
        component.intensity = data["Intensity"].get<f32>();
    }

    inline void from_json(const mag::fs::json& data, game::PerspectiveCameraComponent& component)
    {
        mag::PerspectiveCameraDesc camera_desc = {};
        camera_desc.near = data["Near"].get<f32>();
        camera_desc.far = data["Far"].get<f32>();
        camera_desc.fov = data["Fov"].get<f32>();
        camera_desc.viewport_size = mag::window::get_size();

        component.camera = mag::PerspectiveCamera(camera_desc);
    }

    inline void from_json(const mag::fs::json& data, game::OrthographicCameraComponent& component)
    {
        mag::OrthographicCameraDesc camera_desc = {};
        camera_desc.near = data["Near"].get<f32>();
        camera_desc.far = data["Far"].get<f32>();
        camera_desc.size = data["Size"].get<f32>();
        camera_desc.viewport_size = mag::window::get_size();

        component.camera = mag::OrthographicCamera(camera_desc);
    }

    inline void from_json(const mag::fs::json& data, game::RigidBodyComponent& component)
    {
        component.mass = data["Mass"].get<f32>();

        if (data.contains("BoxCollider"))
        {
            const auto& box_collider = data["BoxCollider"];

            const mag::math::vec3 dimensions = box_collider["Dimensions"].get<mag::math::vec3>();

            component.collider = game::BoxCollider(dimensions);
        }

        else if (data.contains("CapsuleCollider"))
        {
            const auto& capsule_collider = data["CapsuleCollider"];

            const f32 radius = capsule_collider["Radius"].get<f32>();
            const f32 height = capsule_collider["Height"].get<f32>();

            component.collider = game::CapsuleCollider(radius, height);
        }

        else if (data.contains("MeshCollider"))
        {
            const auto& mesh_collider = data["MeshCollider"];

            const str file_path = mesh_collider["FilePath"];

            // @TODO: figure out how to make this async
            const auto& res = mag::resource::get_model(file_path, false);

            std::vector<mag::math::Triangle> triangles;

            for (const auto& m : res->meshes)
            {
                const u32 base_vertex = m.base_vertex;
                const u32 base_index = m.base_index;
                const u32 index_count = m.index_count;

                for (u32 i = base_index; i < base_index + index_count; i += 3)
                {
                    const u32 idx_0 = base_vertex + res->indices[i];
                    const u32 idx_1 = base_vertex + res->indices[i + 1];
                    const u32 idx_2 = base_vertex + res->indices[i + 2];

                    mag::math::Triangle triangle = {};

                    triangle.v0 = res->vertices[idx_0].position;
                    triangle.v1 = res->vertices[idx_1].position;
                    triangle.v2 = res->vertices[idx_2].position;

                    triangles.push_back(triangle);
                }
            }

            component.collider = game::MeshCollider(file_path, triangles);
        }

        else
        {
            MAG_ASSERT(false, "Entity has a rigidbody with no collider");
            component.collider = game::BoxCollider();
        }
    }
};  // namespace nlohmann

namespace game
{
    namespace scene
    {
        b8 save(const str& file_path, Scene& scene)
        {
            mag::fs::Serializer serializer;

            serializer.register_on_save_handler<Scene>(
                [](Scene& scene, mag::fs::Serializer& s)
                {
                    mag::fs::json& data = s.json;

                    // Serialize scene data to file
                    data["Type"] = "Scene";
                    data["Name"] = scene.get_name();

                    auto& ecs = scene.get_ecs();
                    for (const auto entity_id : ecs.get_entities_ids())
                    {
                        mag::fs::json entity;

                        if (auto* component = ecs.get_component<NameComponent>(entity_id))
                        {
                            entity["NameComponent"]["Name"] = component->name;
                        }

                        if (auto* component = ecs.get_component<TransformComponent>(entity_id))
                        {
                            entity["TransformComponent"]["Translation"] = component->translation;
                            entity["TransformComponent"]["Rotation"] = component->rotation;
                            entity["TransformComponent"]["Scale"] = component->scale;
                        }

                        if (auto* component = ecs.get_component<ModelComponent>(entity_id))
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

                        if (auto* component = ecs.get_component<SpriteComponent>(entity_id))
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

                        if (auto* component = ecs.get_component<TextComponent>(entity_id))
                        {
                            entity["TextComponent"]["FilePath"] = component->font->file_path;
                            entity["TextComponent"]["Text"] = component->text;
                            entity["TextComponent"]["Color"] = component->color;
                        }

                        if (auto* component = ecs.get_component<AudioComponent>(entity_id))
                        {
                            entity["AudioComponent"]["FilePath"] = component->audio->file_path;
                            entity["AudioComponent"]["Volume"] = component->volume;
                            entity["AudioComponent"]["PlayOnLoad"] = component->play_on_load;
                            entity["AudioComponent"]["Position"] = component->position;
                            entity["AudioComponent"]["Velocity"] = component->velocity;
                        }

                        if (auto* component = ecs.get_component<RigidBodyComponent>(entity_id))
                        {
                            if (auto* collider = std::get_if<BoxCollider>(&component->collider))
                            {
                                entity["BoxColliderComponent"]["Dimensions"] = collider->dimensions;
                            }

                            if (auto* collider = std::get_if<CapsuleCollider>(&component->collider))
                            {
                                entity["CapsuleColliderComponent"]["Radius"] = collider->radius;
                                entity["CapsuleColliderComponent"]["Height"] = collider->height;
                            }

                            if (auto* collider = std::get_if<MeshCollider>(&component->collider))
                            {
                                entity["MeshColliderComponent"]["FilePath"] = collider->file_path;
                            }

                            entity["RigidBodyComponent"]["Mass"] = component->mass;
                        }

                        if (auto* component = ecs.get_component<LightComponent>(entity_id))
                        {
                            entity["LightComponent"]["Color"] = component->color;
                            entity["LightComponent"]["Intensity"] = component->intensity;
                        }

                        if (auto* component = ecs.get_component<PerspectiveCameraComponent>(entity_id))
                        {
                            entity["PerspectiveCameraComponent"]["Fov"] = component->camera.get_fov();
                            entity["PerspectiveCameraComponent"]["Near"] = component->camera.get_near();
                            entity["PerspectiveCameraComponent"]["Far"] = component->camera.get_far();
                        }

                        if (auto* component = ecs.get_component<OrthographicCameraComponent>(entity_id))
                        {
                            entity["OrthographicCameraComponent"]["Size"] = component->camera.get_size();
                            entity["OrthographicCameraComponent"]["Near"] = component->camera.get_near();
                            entity["OrthographicCameraComponent"]["Far"] = component->camera.get_far();
                        }

                        if (auto* component = ecs.get_component<ScriptComponent>(entity_id))
                        {
                            entity["ScriptComponent"]["FilePath"] = component->file_path;
                        }

                        if (ecs.get_component<DebugComponent>(entity_id) != nullptr)
                        {
                            entity["DebugComponent"] = {};
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

        b8 load(const str& file_path, Scene& scene)
        {
            mag::fs::Serializer serializer;

            serializer.register_on_load_handler<Scene>(
                [file_path](Scene& scene, mag::fs::Serializer& s)
                {
                    mag::fs::json& data = s.json;

                    const str scene_name = data["Name"];
                    scene.set_name(scene_name);
                    scene.set_file_path(file_path);

                    LOG_INFO("Deserializing scene '{0}'", scene_name);

                    if (!data.contains("Entities"))
                    {
                        LOG_WARNING("Scene '{0}' contains no entities", scene.get_name());
                        return;
                    }

                    auto& ecs = scene.get_ecs();

                    for (auto& entity : data["Entities"])
                    {
                        const mag::EntityID entity_id = ecs.create_entity();

                        if (entity.contains("DebugComponent"))
                        {
                            ecs.add_component<DebugComponent>(entity_id, entity["DebugComponent"]);
                        }

                        if (entity.contains("NameComponent"))
                        {
                            ecs.add_component<NameComponent>(entity_id, entity["NameComponent"]);
                        }

                        if (entity.contains("TransformComponent"))
                        {
                            ecs.add_component<TransformComponent>(entity_id, entity["TransformComponent"]);
                        }

                        if (entity.contains("RigidBodyComponent"))
                        {
                            ecs.add_component<RigidBodyComponent>(entity_id, entity["RigidBodyComponent"]);
                        }

                        if (entity.contains("LightComponent"))
                        {
                            ecs.add_component<LightComponent>(entity_id, entity["LightComponent"]);
                        }

                        if (entity.contains("PerspectiveCameraComponent"))
                        {
                            ecs.add_component<PerspectiveCameraComponent>(entity_id,
                                                                          entity["PerspectiveCameraComponent"]);
                        }

                        if (entity.contains("OrthographicCameraComponent"))
                        {
                            ecs.add_component<OrthographicCameraComponent>(entity_id,
                                                                           entity["OrthographicCameraComponent"]);
                        }

                        if (entity.contains("ModelComponent"))
                        {
                            const auto& component = entity["ModelComponent"];
                            const str file_path = component["FilePath"];

                            mag::resource::get_model_async(
                                file_path, scene.get_job_group(),
                                [&ecs, file_path, entity_id](const mag::ref<mag::IResource>& resource)
                                {
                                    auto res = std::dynamic_pointer_cast<mag::ModelResource>(resource);

                                    ecs.add_component<ModelComponent>(entity_id, res);
                                },
                                false);
                        }

                        if (entity.contains("SpriteComponent"))
                        {
                            const auto& component = entity["SpriteComponent"];
                            const str file_path = component["FilePath"];
                            const b8 constant_size = component["ConstantSize"].get<b8>();
                            const b8 always_face_camera = component["AlwaysFaceCamera"].get<b8>();

                            mag::resource::get_texture_async(
                                file_path, scene.get_job_group(),
                                [&ecs, file_path, entity_id, constant_size,
                                 always_face_camera](const mag::ref<mag::IResource>& resource)
                                {
                                    auto res = std::dynamic_pointer_cast<mag::TextureResource>(resource);

                                    ecs.add_component<SpriteComponent>(entity_id, res, constant_size,
                                                                       always_face_camera);
                                },
                                false);
                        }

                        if (entity.contains("TextComponent"))
                        {
                            const auto& component = entity["TextComponent"];
                            const str file_path = component["FilePath"];
                            const str text = component["Text"];
                            const vec4 color = component["Color"].get<vec4>();

                            const auto& font = mag::resource::get_font(file_path);

                            ecs.add_component<TextComponent>(entity_id, font, color, text);
                        }

                        if (entity.contains("AudioComponent"))
                        {
                            const auto& component = entity["AudioComponent"];
                            const str file_path = component["FilePath"];

                            const b8 play_on_load = component["PlayOnLoad"].get<b8>();
                            const f32 volume = component["Volume"].get<f32>();
                            const vec3 position = component["Position"].get<vec3>();
                            const vec3 velocity = component["Velocity"].get<vec3>();

                            const auto& audio = mag::resource::get_audio(file_path);

                            ecs.add_component<AudioComponent>(entity_id, audio, volume, play_on_load, position,
                                                              velocity);
                        }

                        if (entity.contains("ScriptComponent"))
                        {
                            const auto& component = entity["ScriptComponent"];

                            const str file_path = component["FilePath"];

                            ecs.add_component<ScriptComponent>(entity_id, file_path);
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
};  // namespace game
