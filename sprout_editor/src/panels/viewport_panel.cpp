#include "panels/viewport_panel.hpp"

#include <vulkan/vulkan.hpp>

#include "ImGuizmo.h"
#include "backends/imgui_impl_vulkan.h"
#include "camera/camera.hpp"
#include "core/application.hpp"
#include "core/event.hpp"
#include "core/logger.hpp"
#include "ecs/ecs.hpp"
#include "editor.hpp"
#include "editor_scene.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui/misc/cpp/imgui_stdlib.h"
#include "math/generic.hpp"
#include "panels/properties_panel.hpp"
#include "platform/file_system.hpp"
#include "renderer/renderer_image.hpp"
#include "renderer/sampler.hpp"
#include "renderer/shader.hpp"
#include "resources/resource_loader.hpp"
#include "scene/scene_serializer.hpp"
#include "threads/job_system.hpp"
#include "tools/model_importer.hpp"

namespace sprout
{
    struct ViewportPanel::IMPL
    {
            IMPL() = default;
            ~IMPL() = default;

            b8 viewport_window_active = false;
            math::uvec2 viewport_size = {1, 1}, viewport_position = {0, 0};
            vk::DescriptorSet viewport_image_descriptor = {};
            ImGuizmo::OPERATION gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
    };

    ViewportPanel::ViewportPanel() : impl(new IMPL()) {}

    ViewportPanel::~ViewportPanel() = default;

    void ViewportPanel::render(const ImGuiWindowFlags window_flags, const Camera &camera, ECS &ecs,
                               const u32 selected_entity_id, const RendererImage &viewport_image)
    {
        auto &app = get_application();
        auto &editor = get_editor();
        auto &scene = editor.get_active_scene();
        auto &open_scenes = editor.get_open_scenes();

        // Recreate image descriptors
        if (impl->viewport_image_descriptor != nullptr)
        {
            ImGui_ImplVulkan_RemoveTexture(impl->viewport_image_descriptor);
        }

        impl->viewport_image_descriptor =
            ImGui_ImplVulkan_AddTexture(*static_cast<const vk::Sampler *>(viewport_image.get_sampler().get_handle()),
                                        viewport_image.get_image_view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // Runtime controls
        b8 swap_state;
        {
            ImGui::Begin(ICON_FA_GAMEPAD " Runtime Controls", NULL, window_flags);

            // @TODO: make a prettier button
            // @NOTE: keep the runtime switch in the end
            const str button_label = scene.is_running() ? ICON_FA_STOP : ICON_FA_PLAY;
            swap_state = ImGui::Button(button_label.c_str());

            // Recompile shaders
            if (ImGui::Button("Rebuild Shaders"))
            {
                auto &job_system = app.get_job_system();

                // Execute python script on another thread
                auto execute = []
                {
                    b8 result = true;

                    LOG_INFO("Recompiling shaders...");

                    // @TODO: cleanup
                    str configuration = "debug";
#if MAG_CONFIG_PROFILE
                    configuration = "profile";
#elif MAG_CONFIG_RELEASE
                    configuration = "release";
#endif
                    // @TODO: cleanup

                    // @TODO: might be better to recompile only changed shaders, not all of them
                    const str rebuild_script = "python3 build.py shaders " + configuration;
                    if (system(rebuild_script.c_str()) == 0)
                    {
                        LOG_INFO("Finished recompiling shaders");
                    }

                    else
                    {
                        LOG_ERROR("Failed to recompile shaders");
                        result = false;
                    }

                    return result;
                };

                // Callback when finished executing
                auto on_execute_finished = [](const b8 result)
                {
                    // Restart the scene if everything went ok
                    if (result)
                    {
                        auto &app = get_application();
                        auto &shader_manager = app.get_shader_manager();

                        shader_manager.recompile_all_shaders();
                        LOG_SUCCESS("Shaders recompiled");
                    }
                };

                Job load_job = Job(execute, on_execute_finished);
                job_system.add_job(load_job);
            }

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
            static u32 edited_scene_index = Invalid_ID;

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
                    edited_scene_index = Invalid_ID;
                }

                str name = open_scenes[i]->get_name();

                if (edited_scene_index == i && ImGui::InputText("##SceneNameClicked", &name, 0))
                {
                    open_scenes[i]->set_name(name);
                    edited_scene_index = Invalid_ID;
                }

                editor.set_selected_scene_index(i);

                impl->viewport_size = {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};

                ImGui::SetNextItemAllowOverlap();
                ImGui::Image((ImTextureID)((VkDescriptorSet)impl->viewport_image_descriptor),
                             ImVec2(impl->viewport_size.x, impl->viewport_size.y));

                const auto image_pos = ImGui::GetItemRectMin();
                impl->viewport_position = {image_pos.x, image_pos.y};

                // Load assets if any was draged over the viewport
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(CONTENT_BROWSER_ITEM))
                    {
                        auto &app = get_application();

                        const c8 *path = static_cast<const c8 *>(payload->Data);
                        const str extension = fs::get_file_extension(path);

                        ModelImporter importer;

                        // First check if the path exists
                        if (!fs::exists(path))
                        {
                            LOG_ERROR("File not found: {0}", path);
                        }

                        // Then check if its a directory
                        else if (fs::is_directory(path))
                        {
                            LOG_ERROR("Path is a directory: {0}", path);
                        }

                        // Check if asset is a json file
                        else if (extension == ".json")
                        {
                            json data;
                            if (!fs::read_json_data(path, data) || !data.contains("Type"))
                            {
                                goto end_drag_drop_target;
                            }

                            const str type = data["Type"];

                            if (type == "Model")
                            {
                                scene.add_model(path);
                            }

                            else if (type == "Scene")
                            {
                                EditorScene *new_scene = new EditorScene();

                                scene::load(path, *new_scene);

                                editor.add_scene(new_scene);
                            }

                            else if (type == "Material")
                            {
                                // @TODO: make a material viewer or similar
                                // For now, do nothing
                                LOG_WARNING("Asset is a material. No action was performed.");
                            }
                        }

                        // Check if asset is a model that needs to be imported
                        else if (importer.is_extension_supported(extension))
                        {
                            auto &job_system = app.get_job_system();

                            // This is a bit ugly but gets the job done (just don't forget to delete it)
                            str *imported_model_path = new str("");

                            auto on_execute = [path, imported_model_path]
                            {
                                ModelImporter importer;
                                return importer.import(path, *imported_model_path);
                            };

                            auto on_finish = [&scene, imported_model_path](const b8 result)
                            {
                                if (result)
                                {
                                    scene.add_model(*imported_model_path);
                                }

                                delete imported_model_path;
                            };

                            job_system.add_job({on_execute, on_finish});
                        }

                        // Check if asset is an image
                        else if (resource::is_image_extension_supported(extension))
                        {
                            scene.add_sprite(path);
                        }

                        else
                        {
                            LOG_ERROR("Extension not supported: {0}", extension);
                        }
                    }

                end_drag_drop_target:

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
            editor.add_scene(new EditorScene());
        }

        ImGui::EndTabBar();

        // Render gizmos for selected model
        if (!editor.is_disabled() && selected_entity_id != Invalid_ID)
        {
            // Check if entity has a transform before rendering gizmo
            auto *transform = ecs.get_component<TransformComponent>(selected_entity_id);
            if (transform == nullptr)
            {
                goto no_transform;
            }

            mat4 view = camera.get_view();
            const mat4 &proj = camera.get_projection();

            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(impl->viewport_position.x, impl->viewport_position.y, impl->viewport_size.x,
                              impl->viewport_size.y);

            mat4 transform_matrix = transform->get_transformation_matrix();

            if (ImGuizmo::Manipulate(value_ptr(view), value_ptr(proj), impl->gizmo_operation, ImGuizmo::LOCAL,
                                     value_ptr(transform_matrix)))
            {
                vec3 translation, rotation, scale;
                const b8 result = math::decompose_simple(transform_matrix, scale, rotation, translation);

                if (result)
                {
                    transform->translation = translation;
                    transform->rotation = rotation;
                    transform->scale = scale;

                    reset_physics_collider_object(scene, ecs, selected_entity_id);
                }

                else
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
                if (scene.is_running())
                {
                    editor.get_active_scene().on_stop();
                }

                else
                {
                    editor.get_active_scene().on_start();
                }
            }
        }

        impl->viewport_window_active = ImGui::IsWindowFocused();

        ImGui::End();

        ImGui::PopStyleVar();
    }

    void ViewportPanel::on_event(const Event &e)
    {
        dispatch_event<KeyPressEvent>(e, BIND_FN(ViewportPanel::on_key_press));
    }

    void ViewportPanel::on_key_press(const KeyPressEvent &e)
    {
        auto &editor = get_editor();

        if (editor.is_disabled()) return;

        // These are basically the same keybinds as blender
        switch (e.key)
        {
            // Move
            case Key::g:
                impl->gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
                break;

            // Rotate
            case Key::r:
                impl->gizmo_operation = ImGuizmo::OPERATION::ROTATE;
                break;

            // Scale
            case Key::s:
                impl->gizmo_operation = ImGuizmo::OPERATION::SCALE;
                break;

            default:
                break;
        }
    }

    const math::uvec2 &ViewportPanel::get_viewport_size() const { return impl->viewport_size; }

    b8 ViewportPanel::is_viewport_window_active() const { return impl->viewport_window_active; }
};  // namespace sprout
