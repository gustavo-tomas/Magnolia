#include "editor/panels/content_browser_panel.hpp"

#include <filesystem>

#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "editor/editor.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"

namespace mag
{
    ContentBrowserPanel::ContentBrowserPanel()
    {
        folder_image = get_application().get_texture_manager().load("magnolia/assets/images/fa-folder-solid.png",
                                                                    TextureType::Albedo);

        file_image = get_application().get_texture_manager().load("magnolia/assets/images/fa-file-solid.png",
                                                                  TextureType::Albedo);

        folder_image_descriptor =
            ImGui_ImplVulkan_AddTexture(folder_image->get_sampler().get_handle(), folder_image->get_image_view(),
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        file_image_descriptor =
            ImGui_ImplVulkan_AddTexture(file_image->get_sampler().get_handle(), file_image->get_image_view(),
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void ContentBrowserPanel::render(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_FOLDER_OPEN " Content Browser", NULL, window_flags);

        const std::filesystem::path base_directory = std::filesystem::path("magnolia/assets");
        static std::filesystem::path current_directory = base_directory;

        // Traverse directories
        if (current_directory != std::filesystem::path(base_directory))
        {
            if (ImGui::Button(ICON_FA_CHEVRON_LEFT))
            {
                current_directory = current_directory.parent_path();
            }
        }

        static f32 padding = 24.0f;
        static f32 thumbnail_size = 45.0f;
        const f32 cell_size = thumbnail_size + padding;

        const f32 panel_width = ImGui::GetContentRegionAvail().x;
        i32 column_count = panel_width / cell_size;
        if (column_count < 1) column_count = 1;

        ImGui::Columns(column_count, 0, false);

        for (auto &directory_entry : std::filesystem::directory_iterator(current_directory))
        {
            const auto &path = directory_entry.path();
            const str filename_string = path.filename().string();

            ImGui::PushID(filename_string.c_str());
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));  // Remove button background

            if (directory_entry.is_directory())
            {
                ImGui::ImageButton(filename_string.c_str(), folder_image_descriptor, {thumbnail_size, thumbnail_size});
            }

            else
            {
                ImGui::ImageButton(filename_string.c_str(), file_image_descriptor, {thumbnail_size, thumbnail_size});
            }

            if (ImGui::BeginDragDropSource())
            {
                std::filesystem::path relative_path(path);
                const wchar_t *item_path = reinterpret_cast<const wchar_t *>(relative_path.c_str());
                ImGui::SetDragDropPayload(CONTENT_BROWSER_ITEM, item_path, (wcslen(item_path) + 1) * sizeof(wchar_t));
                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (directory_entry.is_directory()) current_directory /= path.filename();
            }
            ImGui::TextWrapped("%s", filename_string.c_str());

            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::Columns(1);

        // ImGui::SliderFloat("Thumbnail Size", &thumbnail_size, 16, 512);
        // ImGui::SliderFloat("Padding", &padding, 0, 32);

        ImGui::End();
    }
};  // namespace mag
