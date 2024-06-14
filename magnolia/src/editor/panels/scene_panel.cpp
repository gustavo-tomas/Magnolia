#include "editor/panels/scene_panel.hpp"

#include "core/application.hpp"
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

            // Pop up menu on right click
            if (selected_entity_id == entity_id && ImGui::BeginPopupContextWindow())
            {
                if (!ecs.get_component<TransformComponent>(selected_entity_id) &&
                    ImGui::MenuItem("Add Transform Component"))
                {
                    ecs.add_component(selected_entity_id, new TransformComponent());
                }

                auto number_of_lights = ecs.get_components<LightComponent>().size();
                if (!ecs.get_component<LightComponent>(selected_entity_id) &&
                    number_of_lights < LightComponent::MAX_NUMBER_OF_LIGHTS && ImGui::MenuItem("Add Light Component"))
                {
                    ecs.add_component(selected_entity_id, new LightComponent());
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Delete"))
                {
                    auto& scene = get_application().get_active_scene();
                    scene.remove_model(entity_id);

                    if (selected_entity_id == entity_id) selected_entity_id = INVALID_ID;
                }

                ImGui::EndPopup();
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
