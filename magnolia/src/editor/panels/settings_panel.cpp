#include "editor/panels/settings_panel.hpp"

#include "core/application.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"

namespace mag
{
    void SettingsPanel::render(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_WRENCH " Settings", NULL, window_flags);

        ImGui::SeparatorText("Scene Settings");
        auto &clear_color = get_application().get_active_scene().get_render_pass().get_clear_color();

        const ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel;
        ImGui::ColorEdit4("Background Color", value_ptr(clear_color), flags);

        ImGui::SeparatorText("Shader Settings");
        ImGui::RadioButton("Show Final Output", reinterpret_cast<i32 *>(&texture_output), 0);
        ImGui::RadioButton("Show Albedo Output", reinterpret_cast<i32 *>(&texture_output), 1);
        ImGui::RadioButton("Show Normal Output", reinterpret_cast<i32 *>(&texture_output), 2);
        ImGui::RadioButton("Show Lighting Output", reinterpret_cast<i32 *>(&texture_output), 3);

        ImGui::Text("Normals");
        ImGui::RadioButton("Use Default Normals", reinterpret_cast<i32 *>(&normal_output), 0);
        ImGui::RadioButton("Use TBN Normals", reinterpret_cast<i32 *>(&normal_output), 1);

        ImGui::End();
    }
};  // namespace mag
