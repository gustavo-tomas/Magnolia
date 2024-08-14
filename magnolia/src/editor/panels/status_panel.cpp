#include "editor/panels/status_panel.hpp"

#include "core/application.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"

namespace mag
{
    void StatusPanel::render(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_INFO " Status", NULL, window_flags);

        auto &app = get_application();
        const auto &context = get_context();

        // Frame times
        {
            // Display frame rate
            const auto &io = ImGui::GetIO();
            const f32 frame_rate = io.Framerate;
            const f32 frame_duration = 1000.0f / frame_rate;

            const Timestamp &timestamp = context.get_timestamp();

            ImGui::SeparatorText("Frame Time");
            ImGui::Text("CPU: %.3f ms/frame - %lf fps", frame_duration, frame_rate);
            ImGui::Text("GPU: %.3f ms/frame", timestamp.end - timestamp.begin);
        }

        // Render passes data
        {
            ImGui::SeparatorText("Render Passes");

            PerformanceResults final_performance_results = {};
            for (const auto *pass : app.get_active_scene().get_render_graph().get_passes())
            {
                const auto &performance = pass->get_performance_results();
                final_performance_results.draw_calls += performance.draw_calls;
                final_performance_results.rendered_triangles += performance.rendered_triangles;

                if (ImGui::CollapsingHeader(pass->get_name().c_str()))
                {
                    ImGui::Text("Triangles: %u", performance.rendered_triangles);
                    ImGui::Text("Draw Calls: %u", performance.draw_calls);
                }
            }

            if (ImGui::CollapsingHeader("Final Results"))
            {
                ImGui::Text("Triangles: %u", final_performance_results.rendered_triangles);
                ImGui::Text("Draw Calls: %u", final_performance_results.draw_calls);
            }
        }

        ImGui::End();
    }
};  // namespace mag
