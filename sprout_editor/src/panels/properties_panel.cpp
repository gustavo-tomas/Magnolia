#include "panels/properties_panel.hpp"

#include "icon_font_cpp/IconsFontAwesome6.h"
#include "renderer/model.hpp"

namespace mag
{
#define MIN_VALUE -1'000'000'000
#define MAX_VALUE +1'000'000'000

    void PropertiesPanel::render(const ImGuiWindowFlags window_flags, ECS &ecs, const u32 selected_entity_id)
    {
        ImGui::Begin(ICON_FA_LIST " Properties", NULL, window_flags);

        // Only render properties if an entity is selected
        if (selected_entity_id == INVALID_ID) goto end;

        // Transform
        if (auto transform = ecs.get_component<TransformComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                editable_field("Translation", transform->translation, vec3(0), vec3(MIN_VALUE), vec3(MAX_VALUE));
                editable_field("Rotation", transform->rotation, vec3(0), vec3(-180), vec3(180));
                editable_field("Scale", transform->scale, vec3(1), vec3(0.0001), vec3(MAX_VALUE));
            }
        }

        // Model
        if (auto model_c = ecs.get_component<ModelComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto &model = model_c->model;

                ImGui::TextWrapped("Name: %s", model->name.c_str());
            }
        }

        // Light
        if (auto light = ecs.get_component<LightComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoAlpha;
                const f32 left_offset = 100.0f;

                ImGui::TextWrapped("Color");
                ImGui::SameLine(left_offset);
                ImGui::ColorEdit4("##Color", value_ptr(light->color), flags);

                editable_field("Intensity", light->intensity, 1.0f, 0.0f, MAX_VALUE);
            }
        }

        // Box collider
        if (auto component = ecs.get_component<BoxColliderComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("BoxCollider", ImGuiTreeNodeFlags_DefaultOpen))
            {
                editable_field("Dimensions", component->dimensions, vec3(1), vec3(0.001), vec3(MAX_VALUE));
            }
        }

        if (auto component = ecs.get_component<RigidBodyComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen))
            {
                editable_field("Mass", component->mass, 1.0f, 0.0f, MAX_VALUE);
            }
        }

        if (auto component = ecs.get_component<CameraComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
            {
                f32 near = component->camera.get_near();
                f32 far = component->camera.get_far();
                f32 fov = component->camera.get_fov();

                editable_field("Near", near, 1.0f, 0.1f, MAX_VALUE);
                editable_field("Far", far, 1.0f, 0.1f, MAX_VALUE);
                editable_field("Fov", fov, 60.0f, 30.0f, 120.0f);

                component->camera.set_near_far({near, far});
                component->camera.set_fov(fov);
            }
        }

        if (auto component = ecs.get_component<ScriptComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::TextWrapped("File Path: %s", component->file_path.c_str());
            }
        }

    end:
        ImGui::End();
    }

    void PropertiesPanel::editable_field(const str &field_name, vec3 &value, const vec3 &reset_value,
                                         const vec3 &min_value, const vec3 &max_value)
    {
        const char *format = "%.2f";
        const f32 left_offset = 100.0f;
        const ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue;

        vec3 editable_value = value;

        ImGui::Text("%s", field_name.c_str());
        ImGui::SameLine(left_offset);

        const str label = str("##") + field_name.c_str();
        if (ImGui::InputFloat3(label.c_str(), value_ptr(editable_value), format, input_flags))
        {
            value = clamp(editable_value, min_value, max_value);
        }

        // Reset
        const str reset_label = str(ICON_FA_CIRCLE) + label;
        ImGui::SameLine();
        if (ImGui::Button(reset_label.c_str()))
        {
            value = reset_value;
        }
    }

    void PropertiesPanel::editable_field(const str &field_name, f32 &value, const f32 reset_value, const f32 min_value,
                                         const f32 max_value)
    {
        const char *format = "%.2f";
        const f32 left_offset = 100.0f;
        const ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue;

        f32 editable_value = value;

        ImGui::Text("%s", field_name.c_str());
        ImGui::SameLine(left_offset);

        const str label = str("##") + field_name.c_str();
        if (ImGui::InputFloat(label.c_str(), &editable_value, 0.0f, 0.0f, format, input_flags))
        {
            value = clamp(editable_value, min_value, max_value);
        }

        // Reset
        const str reset_label = str(ICON_FA_CIRCLE) + label;
        ImGui::SameLine();
        if (ImGui::Button(reset_label.c_str()))
        {
            value = reset_value;
        }
    }
};  // namespace mag