#include "editor/panels/properties_panel.hpp"

#include "icon_font_cpp/IconsFontAwesome6.h"
#include "renderer/model.hpp"

namespace mag
{
    void PropertiesPanel::render(const ImGuiWindowFlags window_flags, ECS &ecs, const u64 selected_entity_id)
    {
        const char *format = "%.2f";
        const ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue;

        ImGui::Begin(ICON_FA_LIST " Properties", NULL, window_flags);

        // Only render properties if an entity is selected
        if (selected_entity_id == INVALID_ID) goto end;

        // Transform
        if (auto transform = ecs.get_component<TransformComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                vec3 translation = transform->translation;
                vec3 rotation = transform->rotation;
                vec3 scale = transform->scale;

                const f32 left_offset = 100.0f;

                ImGui::Text("Translation");
                ImGui::SameLine(left_offset);

                if (ImGui::InputFloat3("##Translation", value_ptr(translation), format, input_flags))
                {
                    transform->translation = translation;
                }

                // Reset
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CIRCLE "##Transform"))
                {
                    transform->translation = vec3(0);
                }

                ImGui::Text("Rotation");
                ImGui::SameLine(left_offset);

                if (ImGui::InputFloat3("##Rotation", value_ptr(rotation), format, input_flags))
                {
                    transform->rotation = rotation;
                }

                // Reset
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CIRCLE "##Scale"))
                {
                    transform->rotation = vec3(0);
                }

                ImGui::Text("Scale");
                ImGui::SameLine(left_offset);

                if (ImGui::InputFloat3("##Scale", value_ptr(scale), format, input_flags))
                {
                    transform->scale = scale;
                }

                // Reset
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CIRCLE "##Rotation"))
                {
                    transform->scale = vec3(1);
                }
            }
        }

        // Model
        if (auto model_c = ecs.get_component<ModelComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto &model = model_c->model;

                ImGui::TextWrapped("Name: %s", model.name.c_str());
            }
        }

        // Light
        if (auto light = ecs.get_component<LightComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto light_intensity = light->intensity;

                const ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoAlpha;

                const f32 left_offset = 100.0f;

                ImGui::TextWrapped("Color");
                ImGui::SameLine(left_offset);
                ImGui::ColorEdit4("##Color", value_ptr(light->color), flags);

                ImGui::Text("Intensity");
                ImGui::SameLine(left_offset);
                if (ImGui::InputFloat("##Intensity", &light_intensity, 1.0f, 10.0f, format, input_flags))
                {
                    light->intensity = light_intensity;
                }
            }
        }

    end:
        ImGui::End();
    }
};  // namespace mag
