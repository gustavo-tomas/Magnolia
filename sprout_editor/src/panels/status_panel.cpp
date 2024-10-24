#include "panels/status_panel.hpp"

#include "editor.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "implot/implot.h"
#include "renderer/context.hpp"
#include "renderer/render_graph.hpp"
#include "tools/profiler.hpp"

namespace sprout
{
    // Helper struct for real time data
    struct ScrollingBuffer
    {
            u32 max_size;
            u32 offset;
            ImVector<ImVec2> data;
            ScrollingBuffer(u32 max_size = 3000) : max_size(max_size), offset(0) { data.reserve(max_size); }

            void add_point(f32 x, f32 y)
            {
                if (data.size() < static_cast<i32>(max_size))
                    data.push_back(ImVec2(x, y));
                else
                {
                    data[offset] = ImVec2(x, y);
                    offset = (offset + 1) % max_size;
                }
            }

            void erase()
            {
                if (data.size() > 0)
                {
                    data.shrink(0);
                    offset = 0;
                }
            }
    };

    StatusPanel::StatusPanel() = default;
    StatusPanel::~StatusPanel() = default;

    void StatusPanel::render(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_INFO " Status", NULL, window_flags);

        auto &editor = get_editor();
        const auto &context = get_context();

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
            ImGui::Text("CPU: %.2f ms/frame (%lf fps)", cpu_timestamp.average, 1000.0 / cpu_timestamp.average);
            ImGui::Text("GPU: %.2f ms/frame", gpu_timestamp.average);

            if (ImGui::CollapsingHeader("Extended Plot"))
            {
                ImPlot::BeginPlot("##Frames");

                static f32 t = 0;
                t += ImGui::GetIO().DeltaTime;

                const f32 history = 5.0f;
                const ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_RangeFit;
                const u32 max_num_of_plots = 100;
                static std::vector<ScrollingBuffer> sdata(max_num_of_plots);

                ImPlot::SetupAxes("time", "ms/frame", flags, flags);
                ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 40);
                ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);

                u32 pos = 0;
                for (const auto &[name, result] : results)
                {
                    sdata[pos].add_point(t, result.average);

                    ImPlot::PlotLine(name.c_str(), &sdata[pos].data[0].x, &sdata[pos].data[0].y, sdata[pos].data.size(),
                                     0, sdata[pos].offset, 2 * sizeof(f32));

                    pos = (pos + 1) % max_num_of_plots;

                    ImGui::Text("[%.2f ms/frame] %s", result.average, name.c_str());
                }

                ImPlot::EndPlot();
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
