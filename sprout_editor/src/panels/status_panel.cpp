#include "panels/status_panel.hpp"

#include "editor.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "tools/profiler.hpp"

namespace sprout
{
    void StatusPanel::render(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_INFO " Status", NULL, window_flags);

        auto &editor = get_editor();
        const auto &context = get_context();

        // @TODO: make a nice graph to show this statistics
        // Frame times
        {
            const auto &profiler = ProfilerManager::get();
            const auto &results = profiler.get_results();

            // GPU timestamp
            const ProfileResult &gpu_timestamp = context.get_timestamp();

            // CPU timestamp
            const ProfileResult &cpu_timestamp =
                results.contains("Application") ? results.at("Application") : ProfileResult{};

            ImGui::SeparatorText("Frame Time");
            ImGui::Text("CPU: %.3f ms/frame (%lf fps)", cpu_timestamp.average, 1000.0 / cpu_timestamp.average);
            ImGui::Text("GPU: %.3f ms/frame", gpu_timestamp.average);

            for (const auto &[name, result] : results)
            {
                ImGui::Text("%s: %.3f ms/frame", name.c_str(), result.average);
            }
        }

        // Render passes data
        {
            ImGui::SeparatorText("Render Passes");

            PerformanceResults final_performance_results = {};
            for (const auto *pass : editor.get_render_graph().get_passes())
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
};  // namespace sprout
