#include "panels/settings_panel.hpp"

#include "icon_font_cpp/IconsFontAwesome6.h"

namespace sprout
{
    SettingsPanel::SettingsPanel() = default;
    SettingsPanel::~SettingsPanel() = default;

    void SettingsPanel::render(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_WRENCH " Settings", NULL, window_flags);

        ImGui::SeparatorText("Shader Settings");
        ImGui::RadioButton("Show Final Output", reinterpret_cast<i32 *>(&texture_output), 0);
        ImGui::RadioButton("Show Albedo Output", reinterpret_cast<i32 *>(&texture_output), 1);
        ImGui::RadioButton("Show Normal Output", reinterpret_cast<i32 *>(&texture_output), 2);
        ImGui::RadioButton("Show Lighting Output", reinterpret_cast<i32 *>(&texture_output), 3);

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
