#include "editor/editor.hpp"

#include <memory>

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "editor/editor_style.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace mag
{
    Editor::Editor(const EventCallback &event_callback) : event_callback(event_callback)
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
        const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
        ImFontConfig icons_config;
        icons_config.MergeMode = true;
        icons_config.PixelSnapH = true;
        icons_config.GlyphMinAdvanceX = icon_font_size;

        const str icon_path = "magnolia/assets/fonts/FontAwesome/" FONT_ICON_FILE_NAME_FAS;
        const str font_path = "magnolia/assets/fonts/Source_Code_Pro/static/SourceCodePro-Regular.ttf";

        io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size);
        io.Fonts->AddFontFromFileTTF(icon_path.c_str(), icon_font_size, &icons_config, icons_ranges);

        set_default_editor_style(ImGui::GetStyle());

        ASSERT(ImGui_ImplSDL2_InitForVulkan(get_application().get_window().get_handle()),
               "Failed to initialize editor window backend");

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = context.get_instance();
        init_info.PhysicalDevice = context.get_physical_device();
        init_info.Device = device;
        init_info.Queue = context.get_graphics_queue();
        init_info.DescriptorPool = static_cast<VkDescriptorPool>(descriptor_pool);
        init_info.MinImageCount = context.get_swapchain_images().size();
        init_info.ImageCount = context.get_swapchain_images().size();
        init_info.UseDynamicRendering = true;
        init_info.ColorAttachmentFormat = static_cast<VkFormat>(render_pass.get_draw_image().get_format());
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ASSERT(ImGui_ImplVulkan_Init(&init_info, nullptr), "Failed to initialize editor renderer backend");

        ASSERT(ImGui_ImplVulkan_CreateFontsTexture(), "Failed to create editor fonts texture");

        content_browser_panel = std::make_unique<ContentBrowserPanel>();
        viewport_panel = std::make_unique<ViewportPanel>();
        info_panel = std::make_unique<InfoPanel>();
        scene_panel = std::make_unique<ScenePanel>();
        material_panel = std::make_unique<MaterialsPanel>();
    }

    Editor::~Editor()
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
        auto &window = get_application().get_window();
        auto &scene = get_application().get_active_scene();

        // Emit event back to the application
        if (viewport_panel->is_resize_needed())
        {
            const auto viewport_size = viewport_panel->get_viewport_size();
            auto event = ViewportResizeEvent(viewport_size.x, viewport_size.y);
            event_callback(event);

            // By now the scene should be updated
            set_viewport_image(scene.get_render_pass().get_target_image());
        }

        // Alternate between Fullscreen and Windowed
        if (window.is_key_pressed(Key::Escape))
        {
            window.set_fullscreen(!window.is_fullscreen());
        }
    }

    void Editor::render(Camera &camera, ECS &ecs)
    {
        auto &context = get_context();
        auto &cmd = context.get_curr_frame().command_buffer;

        viewport_panel->before_render();

        // @TODO: put this inside the render pass?
        render_pass.before_render();
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

        info_panel->render(window_flags);
        content_browser_panel->render(window_flags);
        scene_panel->render(window_flags, ecs, selected_entity_id);
        material_panel->render(window_flags, ecs, selected_entity_id);
        properties_panel->render(window_flags, ecs, selected_entity_id);
        render_settings(window_flags);
        render_camera_properties(window_flags, camera);
        render_status(window_flags);

        ImGui::EndDisabled();

        // @TODO: this feels like a bit of a hack. We keep the viewport with its regular color
        // by rendering after the ImGui::EndDisabled()
        viewport_panel->render(window_flags, camera, ecs, selected_entity_id);

        // End
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd.get_handle());

        cmd.end_rendering();
        render_pass.after_render();

        viewport_panel->after_render();
    }

    void Editor::render_settings(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_PAINTBRUSH " Settings", NULL, window_flags);

        ImGui::SeparatorText("Scene Settings");
        auto &clear_color = get_application().get_active_scene().get_render_pass().get_clear_color();

        const ImGuiColorEditFlags flags = ImGuiColorEditFlags_PickerHueWheel;
        ImGui::ColorEdit4("Background Color", value_ptr(clear_color), flags);

        ImGui::SeparatorText("Shader Settings");
        ImGui::RadioButton("Show Final Output", reinterpret_cast<i32 *>(&texture_output), 0);
        ImGui::RadioButton("Show Albedo Output", reinterpret_cast<i32 *>(&texture_output), 1);
        ImGui::RadioButton("Show Normal Output", reinterpret_cast<i32 *>(&texture_output), 2);
        ImGui::RadioButton("Show Lighting Output", reinterpret_cast<i32 *>(&texture_output), 3);

        ImGui::Text("Normals");
        ImGui::RadioButton("Use Default Normals", reinterpret_cast<i32 *>(&normal_output), 0);
        ImGui::RadioButton("Use TBN Normals", reinterpret_cast<i32 *>(&normal_output), 1);

        ImGui::End();
    }

    void Editor::render_camera_properties(const ImGuiWindowFlags window_flags, Camera &camera)
    {
        ImGui::Begin(ICON_FA_CAMERA " Camera", NULL, window_flags);

        const char *format = "%.2f";
        const f32 left_offset = 100.0f;
        const ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue;

        vec3 translation = camera.get_position();
        vec3 rotation = camera.get_rotation();
        vec2 near_far = camera.get_near_far();
        f32 fov = camera.get_fov();

        ImGui::Text("Translation");
        ImGui::SameLine(left_offset);

        if (ImGui::InputFloat3("##Translation", value_ptr(translation), format, input_flags))
        {
            camera.set_position(translation);
        }

        ImGui::Text("Rotation");
        ImGui::SameLine(left_offset);

        if (ImGui::InputFloat3("##Rotation", value_ptr(rotation), format, input_flags))
        {
            camera.set_rotation(rotation);
        }

        ImGui::Text("FOV");
        ImGui::SameLine(left_offset);

        if (ImGui::InputFloat("##FOV", &fov, 0, 0, format, input_flags))
        {
            camera.set_fov(fov);
        }

        ImGui::Text("Near/Far");
        ImGui::SameLine(left_offset);

        if (ImGui::InputFloat2("##NearFar", value_ptr(near_far), format, input_flags))
        {
            camera.set_near_far(near_far);
        }

        ImGui::End();
    }

    void Editor::render_status(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin(ICON_FA_INFO " Status", NULL, window_flags);

        // Display frame rate
        const auto &io = ImGui::GetIO();
        const f32 frame_rate = io.Framerate;
        const f32 frame_duration = 1000.0f / frame_rate;

        auto &app = get_application();
        const auto &context = get_context();
        const Timestamp &timestamp = context.get_timestamp();
        const Statistics &statistics = app.get_renderer().get_statistics();

        ImGui::Text("CPU: %.3f ms/frame - %lf fps", frame_duration, frame_rate);
        ImGui::Text("GPU: %.3f ms/frame", timestamp.end - timestamp.begin);
        ImGui::Text("Triangles: %u", statistics.rendered_triangles);
        ImGui::Text("Draw Calls: %u", statistics.draw_calls);

        ImGui::End();
    }

    void Editor::on_event(Event &e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<SDLEvent>(BIND_FN(Editor::on_sdl_event));
        dispatcher.dispatch<WindowResizeEvent>(BIND_FN(Editor::on_resize));

        viewport_panel->on_event(e);
    }

    void Editor::on_sdl_event(SDLEvent &e) { ImGui_ImplSDL2_ProcessEvent(&e.e); }

    void Editor::on_resize(WindowResizeEvent &e) { this->render_pass.on_resize({e.width, e.height}); }

    void Editor::set_viewport_image(const Image &viewport_image) { viewport_panel->set_viewport_image(viewport_image); }

    void Editor::set_input_disabled(const b8 disable) { this->disabled = disable; }

    b8 Editor::is_viewport_window_active() const { return viewport_panel->is_viewport_window_active(); }
};  // namespace mag
