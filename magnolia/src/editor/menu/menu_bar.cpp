#include "editor/menu/menu_bar.hpp"

#include "core/types.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"

namespace mag
{
    MenuBar::MenuBar() : info_menu(new InfoMenu()) {}

    void MenuBar::render(const ImGuiWindowFlags window_flags)
    {
        if (ImGui::BeginMainMenuBar())
        {
            // Info panel
            const str name = (str(ICON_FA_CIRCLE_INFO) + " Info").c_str();
            if (ImGui::BeginMenu(name.c_str()))
            {
                info_menu->render(window_flags);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z"))
                {
                    ImGui::Text("@TEST");
                }

                // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Paste", "CTRL+V"))
                {
                    ImGui::Text("@TEST");
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Quit"))
            {
                // @TODO: Emit quit event
                ImGui::Text("@TODO");
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }
};  // namespace mag
