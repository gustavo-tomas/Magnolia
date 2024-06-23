#include "editor/menu/menu_bar.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "scene/scene_serializer.hpp"

namespace mag
{
    MenuBar::MenuBar() : info_menu(new InfoMenu()) {}

    void MenuBar::render(const ImGuiWindowFlags window_flags)
    {
        auto& app = get_application();
        auto& scene = app.get_active_scene();

        // @TODO: shortcuts dont do anything

        if (ImGui::BeginMainMenuBar())
        {
            // File
            if (ImGui::BeginMenu((str(ICON_FA_FILE) + " File").c_str()))
            {
                // Save
                if (ImGui::MenuItem("Save", "CTRL+S"))
                {
                    // @TODO: hardcoded file path
                    const str file_path = "sprout/assets/scenes/test_scene.mag.json";

                    SceneSerializer scene_serializer(scene);
                    scene_serializer.serialize(file_path);

                    LOG_SUCCESS("Saved scene '{0}' to {1}", scene.get_name(), file_path);
                }

                // Load
                // @TODO
                if (ImGui::MenuItem("Open", "CTRL+O"))
                {
                    // @TODO: hardcoded file path
                    const str file_path = "sprout/assets/scenes/test_scene.mag.json";

                    SceneSerializer scene_serializer(scene);
                    scene_serializer.deserialize(file_path);
                }

                ImGui::EndMenu();
            }

            // Info panel
            if (ImGui::BeginMenu((str(ICON_FA_CIRCLE_INFO) + " Info").c_str()))
            {
                info_menu->render(window_flags);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Quit"))
            {
                quit = true;
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
};  // namespace mag
