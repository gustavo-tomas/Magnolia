#include "menu/menu_bar.hpp"

#include <filesystem>

#include "core/application.hpp"
#include "core/logger.hpp"
#include "editor.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui_file_dialog/ImGuiFileDialog.h"
#include "scene/scene_serializer.hpp"

namespace sprout
{
    MenuBar::MenuBar() : info_menu(new InfoMenu()) {}

    void MenuBar::render(const ImGuiWindowFlags window_flags)
    {
        // Display open dialogs
        display_dialog();

        if (ImGui::BeginMainMenuBar())
        {
            // Dont do anything if a dialog is still open
            if (current_action != DialogAction::None)
            {
                ImGui::EndMainMenuBar();
                return;
            }

            // File
            if (ImGui::BeginMenu((str(ICON_FA_FILE) + " File").c_str()))
            {
                // New
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                    new_scene();
                }

                // Save
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    save_active_scene();
                }

                // Save As
                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
                {
                    save_active_scene_as();
                }

                // Load
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                    open_scene();
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
        if (scene_file_path.empty())
        {
            save_active_scene_as();
            return;
        }

        auto& scene = get_editor().get_active_scene();

        SceneSerializer scene_serializer(scene);
        scene_serializer.serialize(scene_file_path);

        LOG_SUCCESS("Saved active scene to '{0}'", scene_file_path);
    }

    void MenuBar::save_active_scene_as()
    {
        IGFD::FileDialogConfig config;
        config.fileName = "untitled.mag.json";

        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Save Scene", ".mag.json", config);
        set_dialog_action(DialogAction::Save);
    }

    void MenuBar::open_scene()
    {
        IGFD::FileDialogConfig config;

        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Open Scene", ".mag.json", config);
        set_dialog_action(DialogAction::Open);
    }

    void MenuBar::new_scene()
    {
        auto* scene = new Scene();

        scene_file_path = std::filesystem::path();

        get_editor().add_scene(scene);

        LOG_SUCCESS("Create new scene '{0}'", scene->get_name());
    }

    void MenuBar::quit_application() { quit = true; }

    void MenuBar::display_dialog()
    {
        auto& editor = get_editor();

        if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                switch (current_action)
                {
                    case DialogAction::Save:
                    {
                        const str file_path = ImGuiFileDialog::Instance()->GetFilePathName();

                        auto& scene = editor.get_active_scene();

                        SceneSerializer scene_serializer(scene);
                        scene_serializer.serialize(file_path);

                        scene_file_path = file_path;

                        LOG_SUCCESS("Saved scene '{0}' to {1}", scene.get_name(), file_path);
                    }
                    break;

                    case DialogAction::Open:
                    {
                        const str file_path = ImGuiFileDialog::Instance()->GetFilePathName();

                        auto* scene = new Scene();

                        SceneSerializer scene_serializer(*scene);
                        scene_serializer.deserialize(file_path);

                        scene_file_path = file_path;

                        // @TODO: Check if scene isnt already opened.

                        editor.add_scene(scene);

                        LOG_SUCCESS("Loaded scene '{0}' from {1}", scene->get_name(), file_path);
                    }
                    break;

                    default:
                        break;
                }
            }

            set_dialog_action(DialogAction::None);
            ImGuiFileDialog::Instance()->Close();
        }
    }

    void MenuBar::on_event(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<KeyPressEvent>(BIND_FN(MenuBar::on_key_press));
    }

    void MenuBar::on_key_press(KeyPressEvent& e)
    {
        auto& window = get_application().get_window();

        const b8 ctrl = window.is_key_down(Keys::Lctrl) || window.is_key_down(Keys::Rctrl);
        const b8 shift = window.is_key_down(Keys::Lshift) || window.is_key_down(Keys::Rshift);

        switch (e.key)
        {
            // New
            case Keys::n:
            {
                if (ctrl)
                {
                    new_scene();
                }
            }
            break;

            // Save
            case Keys::s:
            {
                if (ctrl)
                {
                    if (shift)
                    {
                        save_active_scene_as();
                    }

                    else
                    {
                        save_active_scene();
                    }
                }
            }
            break;

            // Open
            case Keys::o:
            {
                if (ctrl)
                {
                    open_scene();
                }
            }
            break;

            // Quit
            case Keys::q:
            {
                if (ctrl)
                {
                    quit_application();
                }
            }
            break;

            default:
                break;
        }
    }
};  // namespace sprout
