#include "panels/properties_panel.hpp"

#include "ecs/components.hpp"
#include "ecs/ecs.hpp"
#include "editor.hpp"
#include "editor_scene.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui.h"
#include "math/generic.hpp"
#include "misc/cpp/imgui_stdlib.h"
#include "platform/file_system.hpp"
#include "resources/model.hpp"

namespace sprout
{
    const int MinValue = -1'000'000'000;
    const int MaxValue = +1'000'000'000;

    b8 check_file(const str &file_path)
    {
        if (fs::exists(file_path) && !fs::is_directory(file_path))
        {
            return fs::get_file_extension(file_path) == ".cpp";
        }

        return false;
    }

    void reset_physics_collider_object(Scene &scene, ECS &ecs, const u32 entity_id)
    {
        TransformComponent *transform = ecs.get_component<TransformComponent>(entity_id);
        RigidBodyComponent *rigid_body = ecs.get_component<RigidBodyComponent>(entity_id);
        BoxColliderComponent *collider = ecs.get_component<BoxColliderComponent>(entity_id);

        if (rigid_body && collider && transform && !scene.is_running())
        {
            scene.get_physics_world()->reset_rigid_body(rigid_body->collision_object, transform->translation,
                                                        transform->rotation, collider->dimensions, rigid_body->mass);
        }
    }

    PropertiesPanel::PropertiesPanel() = default;
    PropertiesPanel::~PropertiesPanel() = default;

    void PropertiesPanel::render(const ImGuiWindowFlags window_flags, ECS &ecs, const u32 selected_entity_id)
    {
        ImGui::Begin(ICON_FA_LIST " Properties", NULL, window_flags);

        Editor &editor = get_editor();
        EditorScene &scene = editor.get_active_scene();

        // Only render properties if an entity is selected
        if (selected_entity_id == Invalid_ID) goto end;

        // Transform
        if (auto transform = ecs.get_component<TransformComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                b8 field_edited = false;

                vec3 rotation_deg = math::degrees(transform->rotation);

                field_edited = field_edited || editable_field("Translation", transform->translation, vec3(0),
                                                              vec3(MinValue), vec3(MaxValue));

                field_edited = field_edited || editable_field("Rotation", rotation_deg, vec3(0), vec3(-180), vec3(180));

                field_edited =
                    field_edited || editable_field("Scale", transform->scale, vec3(1), vec3(0.0001), vec3(MaxValue));

                if (field_edited && !scene.is_running())
                {
                    reset_physics_collider_object(scene, ecs, selected_entity_id);
                }

                transform->rotation = math::radians(rotation_deg);
            }
        }

        // Model
        if (auto model_c = ecs.get_component<ModelComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto &model = model_c->model;

                ImGui::TextWrapped("Name: %s", model->name.c_str());
            }
        }

        // Sprite
        if (auto component = ecs.get_component<SpriteComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::TextWrapped("File Path: %s", component->texture_file_path.c_str());

                if (ImGui::RadioButton("Face Camera", component->always_face_camera))
                {
                    component->always_face_camera = !component->always_face_camera;
                }

                if (ImGui::RadioButton("Constant Size", component->constant_size))
                {
                    component->constant_size = !component->constant_size;
                }
            }
        }

        // Light
        if (auto light = ecs.get_component<LightComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoAlpha;
                const f32 left_offset = 100.0f;

                ImGui::TextWrapped("Color");
                ImGui::SameLine(left_offset);
                ImGui::ColorEdit4("##Color", value_ptr(light->color), flags);

                editable_field("Intensity", light->intensity, 1.0f, 0.0f, MaxValue);
            }
        }

        // Box collider
        if (auto component = ecs.get_component<BoxColliderComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("BoxCollider", ImGuiTreeNodeFlags_DefaultOpen))
            {
                b8 field_edited =
                    editable_field("Dimensions", component->dimensions, vec3(1), vec3(0.001), vec3(MaxValue));

                if (field_edited && !scene.is_running())
                {
                    reset_physics_collider_object(scene, ecs, selected_entity_id);
                }
            }
        }

        // Rigidbody
        if (auto component = ecs.get_component<RigidBodyComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Rigidbody", ImGuiTreeNodeFlags_DefaultOpen))
            {
                b8 field_edited = editable_field("Mass", component->mass, 1.0f, 0.0f, MaxValue);

                if (field_edited && !scene.is_running())
                {
                    reset_physics_collider_object(scene, ecs, selected_entity_id);
                }
            }
        }

        // Camera
        if (auto component = ecs.get_component<CameraComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
            {
                f32 near = component->camera.get_near();
                f32 far = component->camera.get_far();
                f32 fov = math::degrees(component->camera.get_fov());

                editable_field("Near", near, 1.0f, 0.1f, MaxValue);
                editable_field("Far", far, 1.0f, 0.1f, MaxValue);
                editable_field("Fov", fov, 60.0f, 1.0f, 179.0f);

                component->camera.set_near_far({near, far});
                component->camera.set_fov(math::radians(fov));
            }
        }

        // Script
        if (auto component = ecs.get_component<ScriptComponent>(selected_entity_id))
        {
            if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen))
            {
                const ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackHistory;

                str file_path = component->file_path;

                ImGui::Text("File");
                if (ImGui::InputText("##File", &file_path, flags))
                {
                    // Check file validity
                    if (check_file(file_path))
                    {
                        component->file_path = file_path;
                    }
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(CONTENT_BROWSER_ITEM))
                    {
                        const c8 *file_path = static_cast<const c8 *>(payload->Data);

                        // Check file validity
                        if (check_file(file_path))
                        {
                            // Add new file to file watcher
                            auto &file_watcher = get_application().get_file_watcher();
                            file_watcher.watch_file(file_path);

                            // @NOTE: we dont remove the previous file because the user might want to swap back to it
                            // file_watcher.stop_watching_file(component->file_path);

                            component->file_path = file_path;
                        }
                    }

                    ImGui::EndDragDropTarget();
                }
            }
        }

    end:
        ImGui::End();
    }

    b8 PropertiesPanel::editable_field(const str &field_name, vec3 &value, const vec3 &reset_value,
                                       const vec3 &min_value, const vec3 &max_value) const
    {
        const c8 *format = "%.2f";
        const f32 left_offset = 100.0f;
        const ImGuiInputTextFlags input_flags = 0;

        vec3 editable_value = value;

        ImGui::Text("%s", field_name.c_str());
        ImGui::SameLine(left_offset);

        const str label = str("##") + field_name.c_str();
        const b8 field_changed = ImGui::InputFloat3(label.c_str(), value_ptr(editable_value), format, input_flags);
        if (field_changed)
        {
            value = clamp(editable_value, min_value, max_value);
        }

        // Reset
        const str reset_label = str(ICON_FA_CIRCLE) + label;
        ImGui::SameLine();

        const b8 field_reseted = ImGui::Button(reset_label.c_str());
        if (field_reseted)
        {
            value = reset_value;
        }

        return field_changed || field_reseted;
    }

    b8 PropertiesPanel::editable_field(const str &field_name, f32 &value, const f32 reset_value, const f32 min_value,
                                       const f32 max_value) const
    {
        const c8 *format = "%.2f";
        const f32 left_offset = 100.0f;
        const ImGuiInputTextFlags input_flags = 0;

        f32 editable_value = value;

        ImGui::Text("%s", field_name.c_str());
        ImGui::SameLine(left_offset);

        const str label = str("##") + field_name.c_str();
        const b8 field_changed = ImGui::InputFloat(label.c_str(), &editable_value, 0.0f, 0.0f, format, input_flags);
        if (field_changed)
        {
            value = clamp(editable_value, min_value, max_value);
        }

        // Reset
        const str reset_label = str(ICON_FA_CIRCLE) + label;
        ImGui::SameLine();

        const b8 field_reseted = ImGui::Button(reset_label.c_str());
        if (field_reseted)
        {
            value = reset_value;
        }

        return field_changed || field_reseted;
    }
};  // namespace sprout
