#include "panels/settings_panel.hpp"

#include <vector>

#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui.h"

namespace sprout
{
    SettingsPanel::SettingsPanel() = default;
    SettingsPanel::~SettingsPanel() = default;

    void SettingsPanel::render(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_WRENCH " Settings", NULL, window_flags);

        ImGui::SeparatorText("Shader Settings");

        const std::vector<str> items = {"Final",      "Albedo",  "Normal",    "Roughness", "Metalness",
                                        "Diff (l,n)", "F (l,h)", "G (l,v,h)", "D (h)",     "Specular"};

        ImGui::Text("Shader Output");
        if (ImGui::BeginCombo("##Shader Output", items[texture_output].c_str()))
        {
            for (u32 i = 0; i < items.size(); i++)
            {
                const b8 is_selected = (texture_output == i);
                if (ImGui::Selectable(items[i].c_str(), is_selected))
                {
                    texture_output = i;
                }

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Text("Normals");
        ImGui::RadioButton("Use Default Normals", reinterpret_cast<i32 *>(&normal_output), 0);
        ImGui::RadioButton("Use TBN Normals", reinterpret_cast<i32 *>(&normal_output), 1);

        ImGui::SeparatorText("Scene Settings");
        ImGui::Checkbox("Show Bounding Boxes (AABB)", &enable_bounding_boxes);
        ImGui::Checkbox("Show Physics Colliders", &enable_physics_boxes);

        ImGui::End();
    }

    u32 &SettingsPanel::get_texture_output() { return texture_output; }
    u32 &SettingsPanel::get_normal_output() { return normal_output; }
    b8 &SettingsPanel::is_bounding_box_enabled() { return enable_bounding_boxes; }
    b8 &SettingsPanel::is_physics_colliders_enabled() { return enable_physics_boxes; }
};  // namespace sprout
