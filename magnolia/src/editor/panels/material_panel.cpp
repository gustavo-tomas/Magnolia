#include "editor/panels/material_panel.hpp"

#include "icon_font_cpp/IconsFontAwesome6.h"
#include "renderer/model.hpp"

namespace mag
{
    void MaterialPanel::render(const ImGuiWindowFlags window_flags, ECS &ecs, const u64 selected_entity_id)
    {
        ImGui::Begin(ICON_FA_PAINT_ROLLER " Materials", NULL, window_flags);

        if (selected_entity_id == INVALID_ID) goto end;

        if (auto model_c = ecs.get_component<ModelComponent>(selected_entity_id))
        {
            auto &model = model_c->model;

            for (u32 i = 0; i < model.materials.size(); i++)
            {
                const auto &material = model.materials[i];
                ImGui::Text("Slot %u: %s", i, material->name.c_str());
            }

            const b8 has_material = !model.materials.empty();
            if (!has_material)
            {
                ImGui::Text("No material");
            }
        }

    end:
        ImGui::End();
    }
};  // namespace mag
