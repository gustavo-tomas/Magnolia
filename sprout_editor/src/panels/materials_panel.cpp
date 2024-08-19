#include "panels/materials_panel.hpp"

#include "icon_font_cpp/IconsFontAwesome6.h"
#include "renderer/model.hpp"

namespace sprout
{
    void MaterialsPanel::render(const ImGuiWindowFlags window_flags, ECS &ecs, const u32 selected_entity_id)
    {
        ImGui::Begin(ICON_FA_PAINT_ROLLER " Materials", NULL, window_flags);

        if (selected_entity_id == INVALID_ID) goto end;

        if (auto model_c = ecs.get_component<ModelComponent>(selected_entity_id))
        {
            auto &model = model_c->model;

            for (u32 i = 0; i < model->materials.size(); i++)
            {
                const auto &material = model->materials[i];
                const str slot_str = "Slot " + std::to_string(i) + ": " + material->name.c_str();
                ImGui::SeparatorText(slot_str.c_str());

                ImGui::Text("Textures");
                for (const auto &texture : material->textures)
                {
                    ImGui::TextWrapped("%s", texture->get_name().c_str());
                }
            }

            const b8 has_material = !model->materials.empty();
            if (!has_material)
            {
                ImGui::Text("No material");
            }
        }

    end:
        ImGui::End();
    }
};  // namespace sprout
