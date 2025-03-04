#include "panels/materials_panel.hpp"

#include "core/application.hpp"
#include "ecs/ecs.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui.h"
#include "resources/material.hpp"
#include "resources/model.hpp"

namespace sprout
{
    MaterialsPanel::MaterialsPanel() = default;
    MaterialsPanel::~MaterialsPanel() = default;

    void MaterialsPanel::render(const ImGuiWindowFlags window_flags, ECS &ecs, const u32 selected_entity_id)
    {
        ImGui::Begin(ICON_FA_PAINT_ROLLER " Materials", NULL, window_flags);

        auto &app = get_application();
        auto &material_manager = app.get_material_manager();

        if (selected_entity_id == Invalid_ID) goto end;

        if (auto model_c = ecs.get_component<ModelComponent>(selected_entity_id))
        {
            auto &model = model_c->model;

            for (u32 i = 0; i < model->materials.size(); i++)
            {
                const auto &material = material_manager.get(model->materials[i]);
                const str slot_str = "Slot " + std::to_string(i) + ": " + material->name.c_str();
                ImGui::SeparatorText(slot_str.c_str());

                ImGui::Text("Textures");
                for (const auto &[texture_slot, texture_name] : material->textures)
                {
                    ImGui::TextWrapped("%s", texture_name.c_str());
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
