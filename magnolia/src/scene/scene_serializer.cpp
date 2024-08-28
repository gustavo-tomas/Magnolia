#include "scene/scene_serializer.hpp"

#include <fstream>

#include "core/application.hpp"
#include "core/types.hpp"
#include "ecs/components.hpp"
#include "nlohmann/json.hpp"

namespace mag
{
    using json = nlohmann::ordered_json;

    json& operator<<(json& out, const vec2& v)
    {
        for (i32 i = 0; i < v.length(); i++) out.push_back(v[i]);
        return out;
    }

    json& operator<<(json& out, const vec3& v)
    {
        for (i32 i = 0; i < v.length(); i++) out.push_back(v[i]);
        return out;
    }

    SceneSerializer::SceneSerializer(Scene& scene) : scene(scene) {}

    void SceneSerializer::serialize(const str& file_path)
    {
        std::ofstream file(file_path);

        if (!file.is_open())
        {
            LOG_ERROR("Failed to save scene to file: {0}", file_path);
            return;
        }

        // Serialize scene data to file
        json data;
        data["Scene"] = scene.get_name();

        auto& ecs = scene.get_ecs();
        for (const auto entity_id : ecs.get_entities_ids())
        {
            json entity;

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
                    LOG_WARNING("Model {0} has no file path and will not be serialized", component->model->name);
                }

                else
                {
                    entity["ModelComponent"]["Name"] = component->model->name;
                    entity["ModelComponent"]["FilePath"] = component->model->file_path;
                }
            }

            if (auto component = ecs.get_component<BoxColliderComponent>(entity_id))
            {
                entity["BoxColliderComponent"]["Dimensions"] << component->dimensions;
            }

            if (auto component = ecs.get_component<RigidBodyComponent>(entity_id))
            {
                entity["RigidBodyComponent"]["Mass"] = component->mass;
            }

            if (auto component = ecs.get_component<LightComponent>(entity_id))
            {
                entity["LightComponent"]["Color"] << component->color;
                entity["LightComponent"]["Intensity"] = component->intensity;
            }

            if (auto component = ecs.get_component<CameraComponent>(entity_id))
            {
                entity["CameraComponent"]["Fov"] = component->camera.get_fov();
                entity["CameraComponent"]["Near"] = component->camera.get_near();
                entity["CameraComponent"]["Far"] = component->camera.get_far();
                entity["CameraComponent"]["AspectRatio"] = component->camera.get_aspect_ratio();
            }

            if (auto component = ecs.get_component<ScriptComponent>(entity_id))
            {
                entity["ScriptComponent"]["FilePath"] = component->file_path;
            }

            data["Entities"].push_back(entity);
        }

        file << std::setw(4) << data;
        file.close();
    }

    void SceneSerializer::deserialize(const str& file_path)
    {
        auto& app = get_application();

        // Parse instructions from the json file
        std::ifstream file(file_path);

        if (!file.is_open())
        {
            LOG_ERROR("Failed to open scene from file: {0}", file_path);
            return;
        }

        // Parse the file
        const json data = json::parse(file);

        const str scene_name = data["Scene"];

        LOG_INFO("Deserializing scene '{0}'", scene_name);

        if (data.contains("Entities"))
        {
            auto& ecs = scene.get_ecs();

            for (auto& entity : data["Entities"])
            {
                const u32 entity_id = ecs.create_entity();

                if (entity.contains("NameComponent"))
                {
                    const str entity_name = entity["NameComponent"]["Name"];
                    ecs.get_component<NameComponent>(entity_id)->name = entity_name;
                }

                if (entity.contains("TransformComponent"))
                {
                    const auto& component = entity["TransformComponent"];

                    vec3 translation = vec3(0);
                    vec3 rotation = vec3(0);
                    vec3 scale = vec3(0);

                    // @TODO: dry this
                    for (i32 i = 0; i < translation.length(); i++)
                        translation[i] = component["Translation"][i].get<f32>();

                    for (i32 i = 0; i < rotation.length(); i++) rotation[i] = component["Rotation"][i].get<f32>();

                    for (i32 i = 0; i < scale.length(); i++) scale[i] = component["Scale"][i].get<f32>();

                    ecs.add_component(entity_id, new TransformComponent(translation, rotation, scale));
                }

                if (entity.contains("ModelComponent"))
                {
                    const auto& component = entity["ModelComponent"];
                    const str file_path = component["FilePath"];

                    const auto& model = app.get_model_manager().get(file_path);

                    ecs.add_component(entity_id, new ModelComponent(model));
                }

                if (entity.contains("BoxColliderComponent"))
                {
                    const auto& component = entity["BoxColliderComponent"];

                    vec3 dimensions = vec3(0);

                    for (i32 i = 0; i < dimensions.length(); i++) dimensions[i] = component["Dimensions"][i].get<f32>();

                    ecs.add_component(entity_id, new BoxColliderComponent(dimensions));
                }

                if (entity.contains("RigidBodyComponent"))
                {
                    const auto& component = entity["RigidBodyComponent"];

                    f32 mass = component["Mass"].get<f32>();

                    ecs.add_component(entity_id, new RigidBodyComponent(mass));
                }

                if (entity.contains("LightComponent"))
                {
                    const auto& component = entity["LightComponent"];

                    vec3 color = vec3(0);
                    f32 intensity = 0;

                    for (i32 i = 0; i < color.length(); i++) color[i] = component["Color"][i].get<f32>();
                    intensity = component["Intensity"].get<f32>();

                    ecs.add_component(entity_id, new LightComponent(color, intensity));
                }

                if (entity.contains("CameraComponent"))
                {
                    const auto& component = entity["CameraComponent"];

                    const f32 fov = component["Fov"].get<f32>();
                    const f32 near = component["Near"].get<f32>();
                    const f32 far = component["Far"].get<f32>();
                    const f32 aspect = component["AspectRatio"].get<f32>();

                    Camera camera = Camera(vec3(0), vec3(0), fov, aspect, near, far);

                    ecs.add_component(entity_id, new CameraComponent(camera));
                }

                if (entity.contains("ScriptComponent"))
                {
                    const auto& component = entity["ScriptComponent"];

                    const str file_path = component["FilePath"];

                    ecs.add_component(entity_id, new ScriptComponent(file_path));
                }
            }
        }
    }
};  // namespace mag
