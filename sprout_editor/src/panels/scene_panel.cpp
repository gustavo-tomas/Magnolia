#include "panels/scene_panel.hpp"

#include "ecs/ecs.hpp"
#include "editor.hpp"
#include "editor_scene.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui.h"
#include "resources/model.hpp"

namespace sprout
{
    ScenePanel::ScenePanel() = default;
    ScenePanel::~ScenePanel() = default;

    void ScenePanel::render(const ImGuiWindowFlags window_flags, ECS& ecs)
    {
        // Check if selected id is valid
        if (selected_entity_id != Invalid_ID && !ecs.entity_exists(selected_entity_id))
        {
            selected_entity_id = Invalid_ID;
        }

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
                if (ImGui::MenuItem("New Entity"))
                {
                    ecs.create_entity();
                }
                ImGui::Separator();

                ImGui::Text("Add Component");
                ImGui::Separator();

                b8 enabled = !ecs.get_component<TransformComponent>(selected_entity_id);
                if (ImGui::MenuItem("Add Transform", NULL, false, enabled))
                {
                    ecs.add_component(selected_entity_id, new TransformComponent());
                }

                enabled = !ecs.get_component<BoxColliderComponent>(selected_entity_id);
                if (ImGui::MenuItem("Add BoxCollider", NULL, false, enabled))
                {
                    ecs.add_component(selected_entity_id, new BoxColliderComponent());
                }

                enabled = !ecs.get_component<RigidBodyComponent>(selected_entity_id);
                if (ImGui::MenuItem("Add RigidBody", NULL, false, enabled))
                {
                    ecs.add_component(selected_entity_id, new RigidBodyComponent());
                }

                enabled = !ecs.get_component<CameraComponent>(selected_entity_id);
                if (ImGui::MenuItem("Add Camera", NULL, false, enabled))
                {
                    ecs.add_component(selected_entity_id,
                                      new CameraComponent(Camera(vec3(0), vec3(0), 60.0f, 1.33f, 1.0f, 1000.0f)));
                }

                enabled = !ecs.get_component<LightComponent>(selected_entity_id);
                if (ImGui::MenuItem("Add Light Component", NULL, false, enabled))
                {
                    ecs.add_component(selected_entity_id, new LightComponent());
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Delete"))
                {
                    auto& scene = get_editor().get_active_scene();
                    scene.remove_entity(entity_id);

                    if (selected_entity_id == entity_id) selected_entity_id = Invalid_ID;
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

                if (auto model_c = ecs.get_component<ModelComponent>(entity_id))
                {
                    const str component_name = str(ICON_FA_CUBE) + " " + model_c->model->name;
                    ImGui::Text("%s", component_name.c_str());
                }

                if (ecs.get_component<LightComponent>(entity_id))
                {
                    const str component_name = str(ICON_FA_LIGHTBULB) + " Light";
                    ImGui::Text("%s", component_name.c_str());
                }

                if (ecs.get_component<BoxColliderComponent>(entity_id))
                {
                    const str component_name = str(ICON_FA_VECTOR_SQUARE) + " Box Collider";
                    ImGui::Text("%s", component_name.c_str());
                }

                if (ecs.get_component<RigidBodyComponent>(entity_id))
                {
                    const str component_name = str(ICON_FA_VECTOR_SQUARE) + " Rigidbody";
                    ImGui::Text("%s", component_name.c_str());
                }

                if (ecs.get_component<CameraComponent>(entity_id))
                {
                    const str component_name = str(ICON_FA_CAMERA) + " Camera";
                    ImGui::Text("%s", component_name.c_str());
                }

                if (ecs.get_component<ScriptComponent>(entity_id))
                {
                    const str component_name = str(ICON_FA_SCROLL) + " Script";
                    ImGui::Text("%s", component_name.c_str());
                }

                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

    u32 ScenePanel::get_selected_entity_id() const { return selected_entity_id; }
};  // namespace sprout
