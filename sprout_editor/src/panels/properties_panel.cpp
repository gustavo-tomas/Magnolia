#include "panels/properties_panel.hpp"

#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui/imgui_stdlib.h"
#include "resources/font.hpp"
#include "resources/model.hpp"

namespace sprout
{
#define MIN_VALUE -1'000'000'000
#define MAX_VALUE +1'000'000'000

    template <typename T>
    void editable_field(const str &field_name, T &value, const T &reset_value, const T &min_value, const T &max_value)
    {
        const char *format = "%.3f";
        const f32 left_offset = 100.0f;
        const ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue;

        T editable_value = value;

        ImGui::Text("%s", field_name.c_str());
        ImGui::SameLine(left_offset);

        const str label = str("##") + field_name.c_str();

        // Select input according to the T type

        b8 input = false;

        if constexpr (std::is_same_v<T, f32>)
        {
            input = ImGui::InputFloat(label.c_str(), &editable_value, 0.0f, 0.0f, format, input_flags);
        }

        else if constexpr (std::is_same_v<T, f64>)
        {
            input = ImGui::InputDouble(label.c_str(), &editable_value, 0.0f, 0.0f, format, input_flags);
        }

        else if constexpr (std::is_same_v<T, vec2>)
        {
            input = ImGui::InputFloat2(label.c_str(), value_ptr(editable_value), format, input_flags);
        }

        else if constexpr (std::is_same_v<T, vec3>)
        {
            input = ImGui::InputFloat3(label.c_str(), value_ptr(editable_value), format, input_flags);
        }

        else if constexpr (std::is_same_v<T, vec4>)
        {
            input = ImGui::InputFloat4(label.c_str(), value_ptr(editable_value), format, input_flags);
        }

        if (input)
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
                editable_field<vec3>("Translation", transform->translation, vec3(0), vec3(MIN_VALUE), vec3(MAX_VALUE));
                editable_field<vec3>("Rotation", transform->rotation, vec3(0), vec3(-180), vec3(180));
                editable_field<vec3>("Scale", transform->scale, vec3(1), vec3(0.0001), vec3(MAX_VALUE));
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

        // Sprite
        if (auto component = ecs.get_component<SpriteComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::TextWrapped("File Path: %s", component->texture_file_path.c_str());

                if (ImGui::RadioButton("Face Camera", component->always_face_camera))
                {
                    component->always_face_camera = !component->always_face_camera;
                }

                if (ImGui::RadioButton("Constant Size", component->constant_size))
                {
                    component->constant_size = !component->constant_size;
                }
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

                editable_field<f32>("Intensity", light->intensity, 1.0f, 0.0f, MAX_VALUE);
            }
        }

        // Box collider
        if (auto component = ecs.get_component<BoxColliderComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("BoxCollider", ImGuiTreeNodeFlags_DefaultOpen))
            {
                editable_field<vec3>("Dimensions", component->dimensions, vec3(1), vec3(0.001), vec3(MAX_VALUE));
            }
        }

        // Rigidbody
        if (auto component = ecs.get_component<RigidBodyComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen))
            {
                editable_field<f32>("Mass", component->mass, 1.0f, 0.0f, MAX_VALUE);
            }
        }

        // Camera
        if (auto component = ecs.get_component<CameraComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
            {
                f32 near = component->camera.get_near();
                f32 far = component->camera.get_far();
                f32 fov = component->camera.get_fov();

                editable_field<f32>("Near", near, 1.0f, 0.1f, MAX_VALUE);
                editable_field<f32>("Far", far, 1.0f, 0.1f, MAX_VALUE);
                editable_field<f32>("Fov", fov, 60.0f, 30.0f, 120.0f);

                component->camera.set_near_far({near, far});
                component->camera.set_fov(fov);
            }
        }

        // Text
        if (auto component = ecs.get_component<TextComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Text", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const f32 left_offset = 100.0f;

                ImGui::TextWrapped("Font");
                ImGui::SameLine(left_offset);
                ImGui::Text("%s", component->font->file_path.c_str());

                editable_field<f32>("Kerning", component->kerning, 0.0f, 0.0f, 10.0f);
                editable_field<f32>("Line Spacing", component->line_spacing, 0.0f, 0.0f, 10.0f);

                const ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoAlpha;

                ImGui::TextWrapped("Color");
                ImGui::SameLine(left_offset);
                ImGui::ColorEdit4("##TextColor", value_ptr(component->color), flags);

                str input_str = component->text;
                input_str.resize(1000);

                ImGui::TextWrapped("Text");
                ImGui::SameLine(left_offset);
                if (ImGui::InputTextMultiline("##InputText", &input_str, {0, 0}))
                {
                    component->text = input_str;
                }
            }
        }

        // Script
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
};  // namespace sprout
