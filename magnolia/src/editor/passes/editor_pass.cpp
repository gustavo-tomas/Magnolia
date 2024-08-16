#include "editor/passes/editor_pass.hpp"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "editor/editor.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "renderer/type_conversions.hpp"

namespace mag
{
    EditorPass::EditorPass(const uvec2 &size) : RenderGraphPass("EditorPass", size)
    {
        add_input_attachment("OutputColor", AttachmentType::Color, size);
        add_output_attachment("EditorOutputColor", AttachmentType::Color, size);

        pass.size = size;
        pass.color_clear_value = vec_to_vk_clear_value(vec4(0.1, 0.1, 0.1, 1.0));
        pass.depth_clear_value = {1.0f};
    }

    void EditorPass::on_render(RenderGraph &render_graph)
    {
        auto &context = get_context();
        auto &cmd = context.get_curr_frame().command_buffer;
        auto &scene = get_editor().get_active_scene();
        auto &ecs = scene.get_ecs();
        auto &camera = scene.get_camera();
        auto &viewport_image = render_graph.get_attachment("OutputColor");
        auto &editor = get_editor();

        // Begin
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(get_application().get_window().get_handle());
        ImGui::NewFrame();

        // @NOTE: not very accurate
        performance_results = {};
        performance_results.draw_calls++;
        performance_results.rendered_triangles += 2;

        // Disable widgets
        ImGui::BeginDisabled(editor.disabled);

        const ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode |
                                              static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoWindowMenuButton);

        const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

        // ImGui windows goes here
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dock_flags);
        // ImGui::ShowDemoWindow();

        editor.scene_panel->render(window_flags, ecs);
        const u64 selected_entity_id = editor.scene_panel->get_selected_entity_id();

        editor.menu_bar->render(window_flags);
        editor.content_browser_panel->render(window_flags);
        editor.material_panel->render(window_flags, ecs, selected_entity_id);
        editor.properties_panel->render(window_flags, ecs, selected_entity_id);
        editor.settings_panel->render(window_flags);
        editor.camera_panel->render(window_flags, camera);
        editor.status_panel->render(window_flags);

        ImGui::EndDisabled();

        // @TODO: this feels like a bit of a hack. We keep the viewport with its regular color
        // by rendering after the ImGui::EndDisabled()
        editor.viewport_panel->render(window_flags, camera, ecs, selected_entity_id, viewport_image);

        // End
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd.get_handle());
    }
};  // namespace mag
