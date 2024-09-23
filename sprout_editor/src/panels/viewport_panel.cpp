#include "panels/viewport_panel.hpp"

#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "editor.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

namespace sprout
{
    void ViewportPanel::render(const ImGuiWindowFlags window_flags, const Camera &camera, ECS &ecs,
                               const u32 selected_entity_id, const RendererImage &viewport_image)
    {
        auto &app = get_application();
        auto &editor = get_editor();
        auto &model_loader = app.get_model_loader();
        auto &image_loader = app.get_image_loader();
        auto &font_loader = app.get_font_loader();
        auto &scene = editor.get_active_scene();
        auto &open_scenes = editor.get_open_scenes();

        // Recreate image descriptors
        if (viewport_image_descriptor != nullptr)
        {
            ImGui_ImplVulkan_RemoveTexture(viewport_image_descriptor);
        }

        viewport_image_descriptor =
            ImGui_ImplVulkan_AddTexture(viewport_image.get_sampler().get_handle(), viewport_image.get_image_view(),
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // Runtime controls
        b8 swap_state;
        {
            ImGui::Begin(ICON_FA_GAMEPAD " Runtime Controls", NULL, window_flags);

            const SceneState current_state = scene.get_scene_state();

            // @TODO: make a prettier button
            // @NOTE: keep the runtime switch in the end
            const str button_label = current_state == SceneState::Editor ? ICON_FA_PLAY : ICON_FA_STOP;
            swap_state = ImGui::Button(button_label.c_str());

            ImGui::End();
        }

        // Remove padding for the viewport
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin(ICON_FA_TV " Viewport", NULL, window_flags);

        // Tabs displaying the project scenes

        ImGui::BeginTabBar("Scenes");

        for (u32 i = 0; i < open_scenes.size(); i++)
        {
            const str tab_id = "##" + std::to_string(i);

            // Edit scene name
            static u32 edited_scene_index = INVALID_ID;

            b8 open = true;
            if (ImGui::BeginTabItem((open_scenes[i]->get_name() + tab_id).c_str(), &open))
            {
                // Start editing field
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
                {
                    edited_scene_index = i;
                }

                // Tab changed or lost focus -> quit editing
                if (edited_scene_index != i || !ImGui::IsWindowFocused())
                {
                    edited_scene_index = INVALID_ID;
                }

                str name = open_scenes[i]->get_name();

                if (edited_scene_index == i &&
                    ImGui::InputText("##SceneNameClicked", &name, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    open_scenes[i]->set_name(name);
                    edited_scene_index = INVALID_ID;
                }

                editor.set_selected_scene_index(i);

                viewport_size = {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};

                ImGui::SetNextItemAllowOverlap();
                ImGui::Image(viewport_image_descriptor, ImVec2(viewport_size.x, viewport_size.y));

                // Load assets if any was draged over the viewport
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(CONTENT_BROWSER_ITEM))
                    {
                        auto &app = get_application();
                        auto &file_system = app.get_file_system();

                        const char *path = static_cast<const char *>(payload->Data);
                        const str extension = file_system.get_file_extension(path);

                        // First check if the path exists
                        if (!file_system.exists(path))
                        {
                            LOG_ERROR("File not found: {0}", path);
                        }

                        // Then check if its a directory
                        else if (file_system.is_directory(path))
                        {
                            LOG_ERROR("Path is a directory: {0}", path);
                        }

                        // Check if asset is an image
                        else if (image_loader.is_extension_supported(extension))
                        {
                            scene.add_sprite(path);
                        }

                        // Check if asset is a model
                        else if (model_loader.is_extension_supported(extension))
                        {
                            scene.add_model(path);
                        }

                        // Check if asset is a font
                        else if (font_loader.is_extension_supported(extension))
                        {
                            scene.add_text(path);
                        }

                        else
                        {
                            LOG_ERROR("Extension not supported: {0}", extension);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::EndTabItem();
            }

            // Tab was closed, save the scene (one tab must always stay open).
            if (!open && open_scenes.size() > 1)
            {
                editor.close_scene(open_scenes[i]);
                continue;
            }
        }

        if (ImGui::TabItemButton(ICON_FA_PLUS, ImGuiTabItemFlags_Trailing))
        {
            editor.add_scene(new Scene());
        }

        ImGui::EndTabBar();

        // Render gizmos for selected model
        if (!editor.is_disabled() && selected_entity_id != INVALID_ID)
        {
            // Check if entity has a transform before rendering gizmo
            auto *transform = ecs.get_component<TransformComponent>(selected_entity_id);
            if (transform == nullptr)
            {
                goto no_transform;
            }

            mat4 view = camera.get_view();
            const mat4 &proj = camera.get_projection();

            // Convert from LH to RH coordinates (flip Y)
            const mat4 scale_matrix = scale(mat4(1.0f), vec3(1, -1, 1));
            view = scale_matrix * view;

            auto viewport_min_region = ImGui::GetWindowContentRegionMin();
            auto viewport_max_region = ImGui::GetWindowContentRegionMax();
            auto viewport_offset = ImGui::GetWindowPos();

            vec2 viewport_bounds[2] = {
                {viewport_min_region.x + viewport_offset.x, viewport_min_region.y + viewport_offset.y},
                {viewport_max_region.x + viewport_offset.x, viewport_max_region.y + viewport_offset.y}};

            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(viewport_bounds[0].x, viewport_bounds[0].y, viewport_bounds[1].x - viewport_bounds[0].x,
                              viewport_bounds[1].y - viewport_bounds[0].y);

            mat4 transform_matrix = transform->get_transformation_matrix();

            if (ImGuizmo::Manipulate(value_ptr(view), value_ptr(proj), gizmo_operation, ImGuizmo::LOCAL,
                                     value_ptr(transform_matrix)))
            {
                const b8 result = math::decompose_simple(transform_matrix, transform->scale, transform->rotation,
                                                         transform->translation);

                if (!result)
                {
                    LOG_ERROR("Failed to decompose transformation matrix");
                }
            }
        }

    no_transform:
        // Play/Pause simulation
        {
            if (swap_state)
            {
                if (scene.get_scene_state() == SceneState::Editor)
                {
                    editor.get_active_scene().start_runtime();
                }

                else
                {
                    editor.get_active_scene().stop_runtime();
                }
            }
        }

        viewport_window_active = ImGui::IsWindowFocused();

        ImGui::End();

        ImGui::PopStyleVar();
    }

    void ViewportPanel::on_event(Event &e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<KeyPressEvent>(BIND_FN(ViewportPanel::on_key_press));
    }

    void ViewportPanel::on_key_press(KeyPressEvent &e)
    {
        auto &editor = get_editor();

        if (editor.is_disabled()) return;

        // These are basically the same keybinds as blender
        switch (e.key)
        {
            // Move
            case Key::g:
                gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
                break;

            // Rotate
            case Key::r:
                gizmo_operation = ImGuizmo::OPERATION::ROTATE;
                break;

            // Scale
            case Key::s:
                gizmo_operation = ImGuizmo::OPERATION::SCALE;
                break;

            default:
                break;
        }
    }
};  // namespace sprout
