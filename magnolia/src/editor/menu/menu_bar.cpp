#include "editor/menu/menu_bar.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui_file_dialog/ImGuiFileDialog.h"
#include "scene/scene_serializer.hpp"

namespace mag
{
    b8 is_ctrl_pressed(const ImGuiKey key = ImGuiKey_None)
    {
        if (key != ImGuiKey_None)
        {
            return ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyDown(key);
        }

        return ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
    }

    MenuBar::MenuBar() : info_menu(new InfoMenu()) {}

    void MenuBar::render(const ImGuiWindowFlags window_flags)
    {
        // Save
        if (is_ctrl_pressed(ImGuiKey_S))
        {
            save_active_scene();
        }

        // Quit
        if (is_ctrl_pressed(ImGuiKey_Q))
        {
            quit_application();
        }

        // Display open dialogs
        display_dialog();

        if (ImGui::BeginMainMenuBar())
        {
            // Dont do anything if a dialog is still open
            if (dialog_open)
            {
                ImGui::EndMainMenuBar();
                return;
            }

            // File
            if (ImGui::BeginMenu((str(ICON_FA_FILE) + " File").c_str()))
            {
                // Save
                if (ImGui::MenuItem("Save", "CTRL+S"))
                {
                    save_active_scene();
                }

                // Load
                // @TODO: finish
                if (ImGui::MenuItem("Open", "CTRL+O", false, false))
                {
                    auto* scene = new Scene();

                    // @TODO: hardcoded file path
                    const str file_path = "sprout/assets/scenes/test_scene.mag.json";

                    SceneSerializer scene_serializer(*scene);
                    scene_serializer.deserialize(file_path);
                }

                // Quit
                if (ImGui::MenuItem("Quit", "Ctrl+Q"))
                {
                    quit_application();
                }

                ImGui::EndMenu();
            }

            // Info panel
            if (ImGui::BeginMenu((str(ICON_FA_CIRCLE_INFO) + " Info").c_str()))
            {
                info_menu->render(window_flags);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    void MenuBar::save_active_scene()
    {
        auto& app = get_application();

        IGFD::FileDialogConfig config;
        config.path = "sprout/assets/scenes";
        config.fileName = app.get_active_scene().get_name() + ".mag.json";

        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Save Scene", ".mag.json", config);
        dialog_open = true;
    }

    void MenuBar::quit_application() { quit = true; }

    void MenuBar::display_dialog()
    {
        auto& app = get_application();

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                const str file_path = ImGuiFileDialog::Instance()->GetFilePathName();

                auto& scene = app.get_active_scene();

                SceneSerializer scene_serializer(scene);
                scene_serializer.serialize(file_path);

                LOG_SUCCESS("Saved scene '{0}' to {1}", scene.get_name(), file_path);
            }

            dialog_open = false;
            ImGuiFileDialog::Instance()->Close();
        }
    }
};  // namespace mag
