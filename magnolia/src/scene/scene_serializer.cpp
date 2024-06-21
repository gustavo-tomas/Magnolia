#include "scene/scene_serializer.hpp"

#include <fstream>

#include "core/types.hpp"
#include "ecs/components.hpp"
#include "nlohmann/json.hpp"
#include "renderer/model.hpp"

namespace mag
{
    using json = nlohmann::json;

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

    SceneSerializer::SceneSerializer(BaseScene& scene) : scene(scene) {}

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
                entity["ModelComponent"]["Name"] = component->model.name;
                entity["ModelComponent"]["FilePath"] = component->model.file_path;
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
                entity["LightComponent"]["Intensity"] = component->intensity;
                entity["LightComponent"]["Color"] << component->color;
            }

            data["Entities"].push_back(entity);
        }

        file << std::setw(4) << data;
        file.close();
    }

    void SceneSerializer::deserialize(const str& path)
    {
        (void)path;
        ASSERT(false, "Not implemented");
    }
};  // namespace mag
