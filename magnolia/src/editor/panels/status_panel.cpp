#include "editor/panels/status_panel.hpp"

#include "core/application.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"

namespace mag
{
    void StatusPanel::render(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_INFO " Status", NULL, window_flags);

        // Display frame rate
        const auto &io = ImGui::GetIO();
        const f32 frame_rate = io.Framerate;
        const f32 frame_duration = 1000.0f / frame_rate;

        auto &app = get_application();
        const auto &context = get_context();
        const Timestamp &timestamp = context.get_timestamp();
        const Statistics &statistics = app.get_renderer().get_statistics();

        ImGui::Text("CPU: %.3f ms/frame - %lf fps", frame_duration, frame_rate);
        ImGui::Text("GPU: %.3f ms/frame", timestamp.end - timestamp.begin);
        ImGui::Text("Triangles: %u", statistics.rendered_triangles);
        ImGui::Text("Draw Calls: %u", statistics.draw_calls);

        ImGui::End();
    }
};  // namespace mag
