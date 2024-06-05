#include "editor/panels/scene_panel.hpp"

#include "icon_font_cpp/IconsFontAwesome6.h"
#include "renderer/model.hpp"

namespace mag
{
    void ScenePanel::render(const ImGuiWindowFlags window_flags, ECS& ecs)
    {
        ImGui::Begin(ICON_FA_CUBES " Scene", NULL, window_flags);

        auto entities = ecs.get_entities_ids();
        for (const auto entity_id : entities)
        {
            ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

            if (selected_entity_id == entity_id)
            {
                tree_node_flags |= ImGuiTreeNodeFlags_Selected;
            }

            const str entity_name = ecs.get_component<NameComponent>(entity_id)->name;
            const str node_name = str(ICON_FA_CIRCLE_NOTCH) + " " + entity_name;

            const b8 node_open = ImGui::TreeNodeEx(node_name.c_str(), tree_node_flags);

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                selected_entity_id = entity_id;
            }

            if (node_open)
            {
                if (ecs.get_component<TransformComponent>(entity_id))
                {
                    const str component_name = str(ICON_FA_CIRCLE_NOTCH) + " Transform";
                    ImGui::Text("%s", component_name.c_str());
                }

                if (auto model = ecs.get_component<ModelComponent>(entity_id))
                {
                    const str component_name = str(ICON_FA_CUBE) + " " + model->model.name;
                    ImGui::Text("%s", component_name.c_str());
                }

                if (ecs.get_component<LightComponent>(entity_id))
                {
                    const str component_name = str(ICON_FA_LIGHTBULB) + " Light";
                    ImGui::Text("%s", component_name.c_str());
                }

                ImGui::TreePop();
            }
        }

        ImGui::End();
    }
};  // namespace mag
