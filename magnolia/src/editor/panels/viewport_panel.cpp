#include "editor/panels/viewport_panel.hpp"

#include <filesystem>

#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "editor/editor.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"

namespace mag
{
    void ViewportPanel::before_render()
    {
        // Transition the viewport image into their correct transfer layouts for imgui texture
        if (viewport_image)
        {
            auto &context = get_context();
            auto &cmd = context.get_curr_frame().command_buffer;

            cmd.transfer_layout(viewport_image->get_image(), vk::ImageLayout::eTransferSrcOptimal,
                                vk::ImageLayout::eShaderReadOnlyOptimal);
        }
    }

    void ViewportPanel::render(const ImGuiWindowFlags window_flags, const Camera &camera, ECS &ecs,
                               const u32 selected_entity_id)
    {
        auto &app = get_application();
        auto &editor = app.get_editor();
        auto &model_manager = app.get_model_manager();
        auto &scene = app.get_active_scene();

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

        const uvec2 current_viewport_size = {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};

        if (current_viewport_size != viewport_size)
            resize_needed = true;

        else
            resize_needed = false;

        viewport_size = current_viewport_size;

        ImGui::SetNextItemAllowOverlap();
        ImGui::Image(viewport_image_descriptor, ImVec2(viewport_size.x, viewport_size.y));

        // Load models if any was draged over the viewport
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(CONTENT_BROWSER_ITEM))
            {
                const char *path = static_cast<const char *>(payload->Data);
                const std::filesystem::path file_path(path);
                const str extension = file_path.extension().c_str();

                // First check if the path exists
                if (!std::filesystem::exists(path))
                {
                    LOG_ERROR("File not found: {0}", path);
                }

                // Then check if its a directory
                else if (std::filesystem::is_directory(path))
                {
                    LOG_ERROR("Path is a directory: {0}", path);
                }

                // Then check if this extension is supported
                else if (!model_manager.is_extension_supported(extension))
                {
                    LOG_ERROR("Extension not supported: {0}", extension);
                }

                // Finally load the model
                else
                {
                    scene.add_model(path);
                }
            }
            ImGui::EndDragDropTarget();
        }

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
                    app.get_active_scene().start_runtime();
                }

                else
                {
                    app.get_active_scene().stop_runtime();
                }
            }
        }

        viewport_window_active = ImGui::IsWindowFocused();

        ImGui::End();

        ImGui::PopStyleVar();
    }

    void ViewportPanel::after_render()
    {
        // Return the viewport image to their original layout
        if (viewport_image)
        {
            auto &context = get_context();
            auto &cmd = context.get_curr_frame().command_buffer;

            cmd.transfer_layout(viewport_image->get_image(), vk::ImageLayout::eShaderReadOnlyOptimal,
                                vk::ImageLayout::eTransferSrcOptimal);
        }
    }

    void ViewportPanel::on_event(Event &e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<KeyPressEvent>(BIND_FN(ViewportPanel::on_key_press));
    }

    void ViewportPanel::on_key_press(KeyPressEvent &e)
    {
        auto &editor = get_application().get_editor();

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

    void ViewportPanel::set_viewport_image(const Image &viewport_image)
    {
        // Dont forget to delete old descriptor
        if (viewport_image_descriptor != nullptr) ImGui_ImplVulkan_RemoveTexture(viewport_image_descriptor);

        viewport_image_descriptor =
            ImGui_ImplVulkan_AddTexture(viewport_image.get_sampler().get_handle(), viewport_image.get_image_view(),
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        this->viewport_image = std::addressof(viewport_image);
    }
};  // namespace mag
