#include "editor/editor.hpp"

#include <filesystem>
#include <memory>

#include "IconsFontAwesome5.h"
#include "ImGuizmo.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "core/logger.hpp"
#include "imgui_internal.h"
#include "renderer/model.hpp"

namespace mag
{
    // ImGui drag and drop types
    const char *CONTENT_BROWSER_ITEM = "CONTENT_BROWSER_ITEM";

    void Editor::initialize()
    {
        auto &context = get_context();
        auto &device = context.get_device();

        // Create support structures for command submition
        this->render_pass.initialize({context.get_surface_extent().width, context.get_surface_extent().height});

        std::vector<vk::DescriptorPoolSize> pool_sizes = {{vk::DescriptorType::eSampler, 1000},
                                                          {vk::DescriptorType::eCombinedImageSampler, 1000},
                                                          {vk::DescriptorType::eSampledImage, 1000},
                                                          {vk::DescriptorType::eStorageImage, 1000},
                                                          {vk::DescriptorType::eUniformTexelBuffer, 1000},
                                                          {vk::DescriptorType::eStorageTexelBuffer, 1000},
                                                          {vk::DescriptorType::eUniformBuffer, 1000},
                                                          {vk::DescriptorType::eStorageBuffer, 1000},
                                                          {vk::DescriptorType::eUniformBufferDynamic, 1000},
                                                          {vk::DescriptorType::eStorageBufferDynamic, 1000},
                                                          {vk::DescriptorType::eInputAttachment, 1000}};

        const vk::DescriptorPoolCreateInfo create_info(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1000,
                                                       pool_sizes);

        descriptor_pool = context.get_device().createDescriptorPool(create_info);

        // Initialize imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        auto &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // FontAwesome fonts need to have their sizes reduced by 2/3 in order to align correctly
        const f32 font_size = 16.0f;
        const f32 icon_font_size = font_size * 2.0f / 3.0f;

        // Merge in icons from Font Awesome
        static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
        ImFontConfig icons_config;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.GlyphMinAdvanceX = icon_font_size;

        const str icon_path = str("assets/fonts/FontAwesome/") + FONT_ICON_FILE_NAME_FAS;
        const str font_path = "assets/fonts/Source_Code_Pro/static/SourceCodePro-Regular.ttf";

        io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size);
        io.Fonts->AddFontFromFileTTF(icon_path.c_str(), icon_font_size, &icons_config, icons_ranges);

        this->set_style();

        ASSERT(ImGui_ImplSDL2_InitForVulkan(get_application().get_window().get_handle()),
               "Failed to initialize editor window backend");

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = context.get_instance();
        init_info.PhysicalDevice = context.get_physical_device();
        init_info.Device = device;
        init_info.Queue = context.get_graphics_queue();
        init_info.DescriptorPool = static_cast<VkDescriptorPool>(descriptor_pool);
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        init_info.UseDynamicRendering = true;
        init_info.ColorAttachmentFormat = static_cast<VkFormat>(render_pass.get_draw_image().get_format());
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ASSERT(ImGui_ImplVulkan_Init(&init_info, nullptr), "Failed to initialize editor renderer backend");

        ASSERT(ImGui_ImplVulkan_CreateFontsTexture(), "Failed to create editor fonts texture");

        asset_image = get_application().get_texture_loader().load("assets/images/DefaultAlbedoSeamless.png");

        asset_image_descriptor =
            ImGui_ImplVulkan_AddTexture(asset_image->get_sampler().get_handle(), asset_image->get_image_view(),
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void Editor::shutdown()
    {
        auto &context = get_context();

        this->render_pass.shutdown();

        ImGui_ImplVulkan_DestroyFontsTexture();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        context.get_device().destroyDescriptorPool(descriptor_pool);
    }

    void Editor::update()
    {
        if (resize_needed)
        {
            viewport_resize(viewport_size);
            resize_needed = false;
        }
    }

    void Editor::render(CommandBuffer &cmd, std::vector<Model> &models, const Camera &camera)
    {
        // Transition the draw image into their correct transfer layouts
        cmd.transfer_layout(viewport_image->get_image(), vk::ImageLayout::eTransferSrcOptimal,
                            vk::ImageLayout::eShaderReadOnlyOptimal);

        // @TODO: put this inside the render pass?
        render_pass.before_render(cmd);
        cmd.begin_rendering(this->render_pass.get_pass());

        // Begin
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(get_application().get_window().get_handle());
        ImGui::NewFrame();

        // Disable widgets
        ImGui::BeginDisabled(disabled);

        const ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode |
                                              static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoWindowMenuButton);

        const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

        // ImGui windows goes here
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dock_flags);
        // ImGui::ShowDemoWindow();

        render_panel(window_flags);
        render_content_browser(window_flags);
        render_scene(window_flags, models);

        ImGui::EndDisabled();

        // @TODO: this feels like a bit of a hack. We keep the viewport with its regular color
        // by rendering after the ImGui::EndDisabled()
        render_viewport(window_flags, models, camera);

        // End
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd.get_handle());

        cmd.end_rendering();
        render_pass.after_render(cmd);

        // Return the draw image to their original layout
        cmd.transfer_layout(viewport_image->get_image(), vk::ImageLayout::eShaderReadOnlyOptimal,
                            vk::ImageLayout::eTransferSrcOptimal);
    }

    void Editor::render_content_browser(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_FOLDER_OPEN " Content Browser", NULL, window_flags);

        const std::filesystem::path base_directory = std::filesystem::path("assets");
        static std::filesystem::path current_directory = base_directory;

        // Traverse directories
        if (current_directory != std::filesystem::path(base_directory))
        {
            if (ImGui::Button(ICON_FA_ARROW_LEFT))
            {
                current_directory = current_directory.parent_path();
            }
        }

        static f32 padding = 24.0f;
        static f32 thumbnail_size = 45.0f;
        const f32 cell_size = thumbnail_size + padding;

        const f32 panel_width = ImGui::GetContentRegionAvail().x;
        i32 column_count = panel_width / cell_size;
        if (column_count < 1) column_count = 1;

        ImGui::Columns(column_count, 0, false);

        for (auto &directory_entry : std::filesystem::directory_iterator(current_directory))
        {
            const auto &path = directory_entry.path();
            const str filename_string = path.filename().string();

            ImGui::PushID(filename_string.c_str());
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));  // Remove button background
            ImGui::ImageButton(filename_string.c_str(), asset_image_descriptor, {thumbnail_size, thumbnail_size},
                               {0, 1}, {1, 0});

            if (ImGui::BeginDragDropSource())
            {
                std::filesystem::path relative_path(path);
                const wchar_t *item_path = reinterpret_cast<const wchar_t *>(relative_path.c_str());
                ImGui::SetDragDropPayload(CONTENT_BROWSER_ITEM, item_path, (wcslen(item_path) + 1) * sizeof(wchar_t));
                ImGui::EndDragDropSource();
            }

            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (directory_entry.is_directory()) current_directory /= path.filename();
            }
            ImGui::TextWrapped("%s", filename_string.c_str());

            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::Columns(1);

        ImGui::SliderFloat("Thumbnail Size", &thumbnail_size, 16, 512);
        ImGui::SliderFloat("Padding", &padding, 0, 32);

        ImGui::End();
    }

    void Editor::render_panel(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_INFO_CIRCLE " Panel", NULL, window_flags);

        ImGui::SeparatorText("In any mode");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " Press ESC to enter fullscreen mode");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " Press TAB to alternate between editor and scene views");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " Press KEY_DOWN/KEY_UP to scale image resolution");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " Click PLAY to switch between runtime and editor mode");

        ImGui::SeparatorText("In runtime mode");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " Use WASD and CTRL/ESCAPE to navigate");

        ImGui::SeparatorText("In editor mode");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " Drag and drop assets into the viewport to load them");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " Select a model in the Scene tab to view its properties");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " Press MOUSE_WHEEL and move the mouse to look around");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " Press LEFT_SHIFT and move the mouse to move the camera");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT " Roll MOUSE_WHEEL to zoom in/out");
        ImGui::TextWrapped(ICON_FA_ARROW_ALT_CIRCLE_RIGHT
                           " Use the gizmos to Translate (G), Rotate (R) or Scale (S) the selected model");

        ImGui::End();
    }

    void Editor::render_viewport(const ImGuiWindowFlags window_flags, std::vector<Model> &models, const Camera &camera)
    {
        // Remove padding for the viewport
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin(ICON_FA_TV " Viewport", NULL, window_flags);

        const uvec2 current_viewport_size = {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};

        if (current_viewport_size != viewport_size) resize_needed = true;

        viewport_size = current_viewport_size;

        ImGui::SetNextItemAllowOverlap();
        ImGui::Image(viewport_image_descriptor, ImVec2(viewport_size.x, viewport_size.y));

        // Load models if any was draged over the viewport
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(CONTENT_BROWSER_ITEM))
            {
                const char *path = static_cast<const char *>(payload->Data);
                get_application().add_model(path);
            }
            ImGui::EndDragDropTarget();
        }

        // Position the button - https://github.com/ocornut/imgui/discussions/3862
        ImGuiStyle &style = ImGui::GetStyle();

        // @TODO: this is a bit hardcoded
        const ImVec2 size(ImGui::CalcTextSize(ICON_FA_PLAY).x + style.FramePadding.x * 0.25f,
                          ImGui::CalcTextSize(ICON_FA_PLAY).x + style.FramePadding.x * 4.0f);

        ImGui::SetCursorPos(size);

        ImGui::PushStyleColor(ImGuiCol_Button, 0x00000000);
        ImGui::PushStyleColor(ImGuiCol_Text, 0xff000000);

        // Play/pause button
        static const char *button_icon = ICON_FA_PLAY;
        if (ImGui::Button(button_icon))
        {
            auto &app = get_application();
            auto curr_mode = app.get_active_mode();

            if (curr_mode == Application::Mode::Editor)
            {
                app.set_active_mode(Application::Mode::Runtime);
                button_icon = ICON_FA_PAUSE;
            }

            else
            {
                app.set_active_mode(Application::Mode::Editor);
                button_icon = ICON_FA_PLAY;
            }
        }

        ImGui::PopStyleColor(2);

        // Render gizmos for selected model
        if (!disabled && selected_model_idx != std::numeric_limits<u64>().max())
        {
            auto &model = models[selected_model_idx];

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

            mat4 transform_matrix = Model::get_transformation_matrix(model);

            if (ImGuizmo::Manipulate(value_ptr(view), value_ptr(proj), gizmo_operation, ImGuizmo::LOCAL,
                                     value_ptr(transform_matrix)))
            {
                const b8 result =
                    math::decompose_simple(transform_matrix, model.scale, model.rotation, model.translation);

                if (!result)
                {
                    LOG_ERROR("Failed to decompose transformation matrix");
                }
            }
        }

        ImGui::End();

        ImGui::PopStyleVar();
    }

    void Editor::render_scene(const ImGuiWindowFlags window_flags, std::vector<Model> &models)
    {
        ImGui::Begin(ICON_FA_CUBES " Scene", NULL, window_flags);

        for (u64 i = 0; i < models.size(); i++)
        {
            auto &model = models[i];

            const str node_name = str(ICON_FA_CUBE) + " " + model.name;
            if (ImGui::Selectable(node_name.c_str(), selected_model_idx == i))
            {
                selected_model_idx = i;
            }
        }

        ImGui::End();

        // Only render properties if a model is selected
        if (selected_model_idx != std::numeric_limits<u64>().max())
            render_properties(window_flags, &models[selected_model_idx]);

        else
            render_properties(window_flags, nullptr);
    }

    void Editor::render_properties(const ImGuiWindowFlags window_flags, Model *model)
    {
        ImGui::Begin(ICON_FA_LIST_ALT " Properties", NULL, window_flags);

        if (model != nullptr)
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                vec3 translation = model->translation;
                vec3 rotation = model->rotation;
                vec3 scale = model->scale;

                const f32 left_offset = 100.0f;
                const char *format = "%.2f";
                const ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue;

                ImGui::Text("Translation");
                ImGui::SameLine(left_offset);

                if (ImGui::InputFloat3("##Translation", value_ptr(translation), format, input_flags))
                {
                    model->translation = translation;
                }

                ImGui::Text("Rotation");
                ImGui::SameLine(left_offset);

                if (ImGui::InputFloat3("##Rotation", value_ptr(rotation), format, input_flags))
                {
                    model->rotation = rotation;
                }

                ImGui::Text("Scale");
                ImGui::SameLine(left_offset);

                if (ImGui::InputFloat3("##Scale", value_ptr(scale), format, input_flags))
                {
                    model->scale = scale;
                }
            }
        }

        ImGui::End();
    }

    void Editor::process_events(SDL_Event &e) { ImGui_ImplSDL2_ProcessEvent(&e); }

    void Editor::on_resize(const uvec2 &size) { this->render_pass.on_resize(size); }

    void Editor::on_viewport_resize(std::function<void(const uvec2 &)> callback)
    {
        this->viewport_resize = std::move(callback);
    }

    void Editor::on_key_press(const SDL_Keycode key)
    {
        if (disabled) return;

        // These are basically the same keybinds as blender
        switch (key)
        {
            // Move
            case SDLK_g:
                gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
                break;

            // Rotate
            case SDLK_r:
                gizmo_operation = ImGuizmo::OPERATION::ROTATE;
                break;

            // Scale
            case SDLK_s:
                gizmo_operation = ImGuizmo::OPERATION::SCALE;
                break;
        }
    }

    void Editor::set_viewport_image(const Image &viewport_image)
    {
        // Dont forget to delete old descriptor
        if (viewport_image_descriptor != nullptr) ImGui_ImplVulkan_RemoveTexture(viewport_image_descriptor);

        viewport_image_descriptor =
            ImGui_ImplVulkan_AddTexture(viewport_image.get_sampler().get_handle(), viewport_image.get_image_view(),
                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        this->viewport_image = std::addressof(viewport_image);
    }

    void Editor::set_input_disabled(const b8 disable) { this->disabled = disable; }

    void Editor::set_style()
    {
        ImGuiStyle &style = ImGui::GetStyle();

        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.07f;
        style.WindowRounding = 3.0f;
        style.WindowBorderSize = 0.0f;
        style.WindowMinSize = {150.0f, 100.0f};
        style.FrameRounding = 3.0f;
        style.FrameBorderSize = 0.0f;
        style.FramePadding = ImVec2(6.0f, 4.0f);
        style.SeparatorTextPadding.y = style.FramePadding.y;
        style.TabRounding = 1.0f;
        style.TabBorderSize = 0.0f;
        style.TabBarBorderSize = 0.0f;
        style.DockingSeparatorSize = 1.0f;

        const ImVec4 black_opaque = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        const ImVec4 white_opaque = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        const ImVec4 gray_opaque = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
        const ImVec4 blue_opaque = ImVec4(0.03f, 0.124f, 0.315f, 1.00f);

        style.Colors[ImGuiCol_Text] = white_opaque;
        // style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        // style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_WindowBg] = black_opaque;
        // style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
        style.Colors[ImGuiCol_Border] = gray_opaque;
        // style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, .00f, 1.00f, 1.0f);
        // style.Colors[ImGuiCol_FrameBg] = white_opaque;
        // style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        // style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = gray_opaque;
        // style.Colors[ImGuiCol_TitleBgCollapsed] = white_opaque;
        style.Colors[ImGuiCol_TitleBgActive] = blue_opaque;
        style.Colors[ImGuiCol_MenuBarBg] = black_opaque;
        style.Colors[ImGuiCol_Tab] = gray_opaque;
        style.Colors[ImGuiCol_TabHovered] = blue_opaque;
        style.Colors[ImGuiCol_TabActive] = blue_opaque;
        style.Colors[ImGuiCol_TabUnfocused] = gray_opaque;
        style.Colors[ImGuiCol_TabUnfocusedActive] = gray_opaque;
        style.Colors[ImGuiCol_ScrollbarBg] = black_opaque;
        // style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
        // style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
        // style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
        // style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        // style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        // style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        // style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        // style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        // style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Header] = blue_opaque;
        // style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        // style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.926f, 0.059f, 0.098f, 1.00f);
        // style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
        // style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        // style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        // style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        // style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        // style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        // style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    }
};  // namespace mag
