#include "editor/panels/camera_panel.hpp"

#include "icon_font_cpp/IconsFontAwesome6.h"

namespace mag
{
    void CameraPanel::render(const ImGuiWindowFlags window_flags, Camera& camera)
    {
        ImGui::Begin(ICON_FA_CAMERA " Camera", NULL, window_flags);

        const char* format = "%.2f";
        const f32 left_offset = 100.0f;
        const ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue;

        vec3 translation = camera.get_position();
        vec3 rotation = camera.get_rotation();
        vec2 near_far = camera.get_near_far();
        f32 fov = camera.get_fov();

        ImGui::Text("Translation");
        ImGui::SameLine(left_offset);

        if (ImGui::InputFloat3("##Translation", value_ptr(translation), format, input_flags))
        {
            camera.set_position(translation);
        }

        ImGui::Text("Rotation");
        ImGui::SameLine(left_offset);

        if (ImGui::InputFloat3("##Rotation", value_ptr(rotation), format, input_flags))
        {
            camera.set_rotation(rotation);
        }

        ImGui::Text("FOV");
        ImGui::SameLine(left_offset);

        if (ImGui::InputFloat("##FOV", &fov, 0, 0, format, input_flags))
        {
            camera.set_fov(fov);
        }

        ImGui::Text("Near/Far");
        ImGui::SameLine(left_offset);

        if (ImGui::InputFloat2("##NearFar", value_ptr(near_far), format, input_flags))
        {
            camera.set_near_far(near_far);
        }

        ImGui::End();
    }
};  // namespace mag
