#include "menu/info_menu.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"

namespace sprout
{
    using namespace mag;

    void InfoMenu::render(const ImGuiWindowFlags window_flags)
    {
        // Parse instructions from the json file
        const str file_path = "sprout_editor/assets/json/editor_instructions.json";

        auto& app = get_application();
        auto& file_system = app.get_file_system();

        json data;

        if (!file_system.read_json_data(file_path, data))
        {
            LOG_ERROR("Failed to open editor instructions from json file: {0}", file_path);
            return;
        }

        // Read the instructions
        const str window_name = data["window_name"];
        ImGui::Begin((str(ICON_FA_CIRCLE_INFO) + " " + window_name).c_str(), NULL, window_flags);

        for (const auto& section : data["sections"])
        {
            const str section_name = section["name"];
            ImGui::SeparatorText(section_name.c_str());

            for (const auto& instruction : section["instructions"])
            {
                const str instruction_name = instruction;
                ImGui::TextWrapped("%s", (str(ICON_FA_CIRCLE_ARROW_RIGHT) + " " + instruction_name).c_str());
            }
        }

        ImGui::End();
    }
};  // namespace sprout
