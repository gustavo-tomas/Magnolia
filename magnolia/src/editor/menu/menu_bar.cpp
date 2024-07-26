#include "editor/menu/menu_bar.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui_file_dialog/ImGuiFileDialog.h"
#include "scene/scene_serializer.hpp"

namespace mag
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
                // Save
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                    save_active_scene();
                }

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
        auto& app = get_application();

        if (scene_file_path.empty())
        {
            save_active_scene_as();
            return;
        }

        auto& scene = app.get_active_scene();

        SceneSerializer scene_serializer(scene);
        scene_serializer.serialize(scene_file_path);

        LOG_SUCCESS("Saved active scene to '{0}'", scene_file_path);
    }

    void MenuBar::save_active_scene_as()
    {
        IGFD::FileDialogConfig config;
        config.fileName = "untitled.mag.json";

        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Save Scene", ".mag.json", config);
        current_action = DialogAction::Save;
    }

    // @TODO: finish
    void MenuBar::open_scene()
    {
        auto* scene = new Scene();

        // @TODO: hardcoded file path
        const str file_path = "sprout/assets/scenes/sponza_scene.mag.json";

        SceneSerializer scene_serializer(*scene);
        scene_serializer.deserialize(file_path);

        get_application().enqueue_scene(scene);
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

                scene_file_path = file_path;

                LOG_SUCCESS("Saved scene '{0}' to {1}", scene.get_name(), file_path);
            }

            current_action = DialogAction::None;
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
};  // namespace mag
