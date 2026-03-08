#include "scene_serializer.hpp"

#include <magnolia/camera/camera.hpp>
#include <magnolia/core/assert.hpp>
#include <magnolia/ecs/ecs.hpp>
#include <magnolia/physics/physics.hpp>
#include <magnolia/platform/file_system.hpp>
#include <magnolia/platform/serializer.hpp>
#include <magnolia/platform/window.hpp>
#include <magnolia/resources/audio.hpp>
#include <magnolia/resources/font.hpp>
#include <magnolia/resources/model.hpp>
#include <magnolia/resources/resource.hpp>
#include <magnolia/resources/texture.hpp>

#include "ecs/components.hpp"
#include "scene.hpp"

namespace game
{
    static mag::fs::json& operator<<(mag::fs::json& out, const mag::vec2& v)
    {
        for (i32 i = 0; i < v.length(); i++)
        {
            out.push_back(v[i]);
        }
        return out;
    }

    static mag::fs::json& operator<<(mag::fs::json& out, const mag::vec3& v)
    {
        for (i32 i = 0; i < v.length(); i++)
        {
            out.push_back(v[i]);
        }
        return out;
    }

    static mag::fs::json& operator<<(mag::fs::json& out, const mag::vec4& v)
    {
        for (i32 i = 0; i < v.length(); i++)
        {
            out.push_back(v[i]);
        }
        return out;
    }

    static mag::fs::json& operator<<(mag::fs::json& out, const mag::quat& q)
    {
        for (i32 i = 0; i < q.length(); i++)
        {
            out.push_back(q[i]);
        }
        return out;
    }

    static void get_array_value(const mag::fs::json& data, mag::math::vec3& v)
    {
        for (i32 i = 0; i < mag::math::vec3::length(); i++)
        {
            v[i] = data[i].get<f32>();
        }
    }

    static void get_array_value(const mag::fs::json& data, mag::math::vec4& v)
    {
        for (i32 i = 0; i < mag::math::vec4::length(); i++)
        {
            v[i] = data[i].get<f32>();
        }
    }

    static void get_array_value(const mag::fs::json& data, mag::math::quat& v)
    {
        for (i32 i = 0; i < mag::math::quat::length(); i++)
        {
            v[i] = data[i].get<f32>();
        }
    }

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

                        if (auto component = ecs.get_component<NameComponent>(entity_id))
                        {
                            entity["NameComponent"]["Name"] = component->name;
                        }

                        if (auto component = ecs.get_component<TransformComponent>(entity_id))
                        {
                            entity["TransformComponent"]["Translation"] << component->translation;
                            entity["TransformComponent"]["Rotation"] << component->rotation;
                            entity["TransformComponent"]["Scale"] << component->scale;
                        }

                        if (auto component = ecs.get_component<ModelComponent>(entity_id))
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

                        if (auto component = ecs.get_component<SpriteComponent>(entity_id))
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

                        if (auto component = ecs.get_component<TextComponent>(entity_id))
                        {
                            entity["TextComponent"]["FilePath"] = component->font->file_path;
                            entity["TextComponent"]["Text"] = component->text;
                            entity["TextComponent"]["Color"] << component->color;
                        }

                        if (auto component = ecs.get_component<AudioComponent>(entity_id))
                        {
                            entity["AudioComponent"]["FilePath"] = component->audio->file_path;
                            entity["AudioComponent"]["Volume"] = component->volume;
                            entity["AudioComponent"]["PlayOnLoad"] = component->play_on_load;
                            entity["AudioComponent"]["Position"] << component->position;
                            entity["AudioComponent"]["Velocity"] << component->velocity;
                        }

                        if (auto component = ecs.get_component<RigidBodyComponent>(entity_id))
                        {
                            if (auto* collider = std::get_if<BoxCollider>(&component->collider))
                            {
                                entity["BoxColliderComponent"]["Dimensions"] << collider->dimensions;
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

                        if (auto component = ecs.get_component<LightComponent>(entity_id))
                        {
                            entity["LightComponent"]["Color"] << component->color;
                            entity["LightComponent"]["Intensity"] = component->intensity;
                        }

                        if (auto component = ecs.get_component<PerspectiveCameraComponent>(entity_id))
                        {
                            entity["PerspectiveCameraComponent"]["Fov"] = component->camera.get_fov();
                            entity["PerspectiveCameraComponent"]["Near"] = component->camera.get_near();
                            entity["PerspectiveCameraComponent"]["Far"] = component->camera.get_far();
                        }

                        if (auto component = ecs.get_component<OrthographicCameraComponent>(entity_id))
                        {
                            entity["OrthographicCameraComponent"]["Size"] = component->camera.get_size();
                            entity["OrthographicCameraComponent"]["Near"] = component->camera.get_near();
                            entity["OrthographicCameraComponent"]["Far"] = component->camera.get_far();
                        }

                        if (auto component = ecs.get_component<ScriptComponent>(entity_id))
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
                        return;
                    }

                    auto& ecs = scene.get_ecs();

                    for (auto& entity : data["Entities"])
                    {
                        const mag::EntityID entity_id = ecs.create_entity();

                        if (entity.contains("NameComponent"))
                        {
                            const str entity_name = entity["NameComponent"]["Name"];
                            ecs.add_component<NameComponent>(entity_id, entity_name);
                        }

                        if (entity.contains("TransformComponent"))
                        {
                            const auto& component = entity["TransformComponent"];

                            mag::vec3 translation = mag::vec3(0);
                            mag::quat rotation = mag::quat(1.0f, 0.0f, 0.0f, 0.0f);
                            mag::vec3 scale = mag::vec3(0);

                            get_array_value(component["Translation"], translation);
                            get_array_value(component["Rotation"], rotation);
                            get_array_value(component["Scale"], scale);

                            ecs.add_component<TransformComponent>(entity_id, translation, rotation, scale);
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
                            mag::vec4 color = mag::vec4(0.0f);

                            get_array_value(component["Color"], color);

                            const auto& font = mag::resource::get_font(file_path);

                            ecs.add_component<TextComponent>(entity_id, font, color, text);
                        }

                        if (entity.contains("AudioComponent"))
                        {
                            const auto& component = entity["AudioComponent"];
                            const str file_path = component["FilePath"];

                            const f32 volume = component["Volume"].get<f32>();
                            const b8 play_on_load = component["PlayOnLoad"].get<f32>();
                            mag::vec3 position = mag::vec3(0.0f);
                            mag::vec3 velocity = mag::vec3(0.0f);

                            get_array_value(component["Position"], position);
                            get_array_value(component["Velocity"], velocity);

                            const auto& audio = mag::resource::get_audio(file_path);

                            ecs.add_component<AudioComponent>(entity_id, audio, volume, play_on_load, position,
                                                              velocity);
                        }

                        if (entity.contains("RigidBodyComponent"))
                        {
                            const auto& component = entity["RigidBodyComponent"];

                            f32 mass = component["Mass"].get<f32>();

                            Collider collider = BoxCollider();

                            if (component.contains("BoxCollider"))
                            {
                                const auto& box_collider = component["BoxCollider"];

                                mag::vec3 dimensions = mag::vec3(0);

                                get_array_value(box_collider["Dimensions"], dimensions);

                                collider = BoxCollider(dimensions);
                            }

                            else if (component.contains("CapsuleCollider"))
                            {
                                const auto& capsule_collider = component["CapsuleCollider"];

                                const f32 radius = capsule_collider["Radius"].get<f32>();
                                const f32 height = capsule_collider["Height"].get<f32>();

                                collider = CapsuleCollider(radius, height);
                            }

                            else if (component.contains("MeshCollider"))
                            {
                                const auto& mesh_collider = component["MeshCollider"];

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

                                collider = MeshCollider(file_path, triangles);
                            }

                            else
                            {
                                MAG_ASSERT(false, "Entity '{0}' has a rigidbody with no collider", entity_id);
                            }

                            ecs.add_component<RigidBodyComponent>(entity_id, collider, mass);
                        }

                        if (entity.contains("LightComponent"))
                        {
                            const auto& component = entity["LightComponent"];

                            mag::vec3 color = mag::vec3(0);
                            f32 intensity = 0;

                            get_array_value(component["Color"], color);
                            intensity = component["Intensity"].get<f32>();

                            ecs.add_component<LightComponent>(entity_id, color, intensity);
                        }

                        if (entity.contains("PerspectiveCameraComponent"))
                        {
                            const auto& component = entity["PerspectiveCameraComponent"];

                            const f32 fov = component["Fov"].get<f32>();
                            const f32 near = component["Near"].get<f32>();
                            const f32 far = component["Far"].get<f32>();

                            mag::PerspectiveCameraDesc camera_desc = {};
                            camera_desc.near = near;
                            camera_desc.far = far;
                            camera_desc.fov = fov;
                            camera_desc.viewport_size = mag::window::get_size();

                            mag::PerspectiveCamera camera = mag::PerspectiveCamera(camera_desc);

                            ecs.add_component<PerspectiveCameraComponent>(entity_id, camera);
                        }

                        if (entity.contains("OrthographicCameraComponent"))
                        {
                            const auto& component = entity["OrthographicCameraComponent"];

                            const f32 size = component["Size"].get<f32>();
                            const f32 near = component["Near"].get<f32>();
                            const f32 far = component["Far"].get<f32>();

                            mag::OrthographicCameraDesc camera_desc = {};
                            camera_desc.near = near;
                            camera_desc.far = far;
                            camera_desc.size = size;
                            camera_desc.viewport_size = mag::window::get_size();

                            mag::OrthographicCamera camera = mag::OrthographicCamera(camera_desc);

                            ecs.add_component<OrthographicCameraComponent>(entity_id, camera);
                        }

                        if (entity.contains("ScriptComponent"))
                        {
                            const auto& component = entity["ScriptComponent"];

                            const str file_path = component["FilePath"];

                            ecs.add_component<ScriptComponent>(entity_id, file_path);
                        }

                        if (entity.contains("DebugComponent"))
                        {
                            ecs.add_component<DebugComponent>(entity_id);
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
