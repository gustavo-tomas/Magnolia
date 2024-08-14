#include "editor/editor.hpp"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "editor/editor_style.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "renderer/type_conversions.hpp"

namespace mag
{
    Editor::Editor(const EventCallback &event_callback) : event_callback(event_callback)
    {
        auto &context = get_context();
        auto &device = context.get_device();

        // Create support structures for command submition

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
        init_info.ColorAttachmentFormat = static_cast<VkFormat>(vk::Format::eR16G16B16A16Sfloat);
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ASSERT(ImGui_ImplVulkan_Init(&init_info, nullptr), "Failed to initialize editor renderer backend");

        ASSERT(ImGui_ImplVulkan_CreateFontsTexture(), "Failed to create editor fonts texture");

        menu_bar = std::make_unique<MenuBar>();
        content_browser_panel = std::make_unique<ContentBrowserPanel>();
        viewport_panel = std::make_unique<ViewportPanel>();
        scene_panel = std::make_unique<ScenePanel>();
        material_panel = std::make_unique<MaterialsPanel>();
        status_panel = std::make_unique<StatusPanel>();
        camera_panel = std::make_unique<CameraPanel>();
        settings_panel = std::make_unique<SettingsPanel>();
    }

    Editor::~Editor()
    {
        auto &context = get_context();

        ImGui_ImplVulkan_DestroyFontsTexture();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        context.get_device().destroyDescriptorPool(descriptor_pool);
    }

    void Editor::update()
    {
        auto &window = get_application().get_window();

        // Emit event back to the application
        if (viewport_panel->is_resize_needed())
        {
            const auto viewport_size = viewport_panel->get_viewport_size();
            auto event = ViewportResizeEvent(viewport_size.x, viewport_size.y);
            event_callback(event);
        }

        if (menu_bar->quit_requested())
        {
            auto event = QuitEvent();
            event_callback(event);
        }

        // Alternate between Fullscreen and Windowed
        if (window.is_key_pressed(Key::Escape))
        {
            window.set_fullscreen(!window.is_fullscreen());
        }
    }

    void Editor::on_event(Event &e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<SDLEvent>(BIND_FN(Editor::on_sdl_event));

        menu_bar->on_event(e);
        viewport_panel->on_event(e);
    }

    void Editor::on_sdl_event(SDLEvent &e) { ImGui_ImplSDL2_ProcessEvent(&e.e); }

    void Editor::set_input_disabled(const b8 disable) { this->disabled = disable; }

    b8 Editor::is_viewport_window_active() const { return viewport_panel->is_viewport_window_active(); }

    // EditorPass ------------------------------------------------------------------------------------------------------

    EditorPass::EditorPass(const uvec2 &size) : RenderGraphPass("EditorPass", size)
    {
        add_input_attachment("OutputColor", AttachmentType::Color, size);
        add_output_attachment("EditorOutputColor", AttachmentType::Color, size);

        pass.size = size;
        pass.color_clear_value = vec_to_vk_clear_value(vec4(0.1, 0.1, 0.1, 1.0));
        pass.depth_clear_value = {1.0f};
    }

    void EditorPass::on_render(RenderGraph &render_graph)
    {
        auto &app = get_application();
        auto &editor = app.get_editor();
        auto &context = get_context();
        auto &cmd = context.get_curr_frame().command_buffer;
        auto &scene = app.get_active_scene();
        auto &ecs = scene.get_ecs();
        auto &camera = scene.get_camera();
        auto &viewport_image = render_graph.get_attachment("OutputColor");

        // Begin
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(get_application().get_window().get_handle());
        ImGui::NewFrame();

        // Disable widgets
        ImGui::BeginDisabled(editor.disabled);

        const ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode |
                                              static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoWindowMenuButton);

        const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

        // ImGui windows goes here
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dock_flags);
        // ImGui::ShowDemoWindow();

        editor.scene_panel->render(window_flags, ecs);
        const u64 selected_entity_id = editor.scene_panel->get_selected_entity_id();

        editor.menu_bar->render(window_flags);
        editor.content_browser_panel->render(window_flags);
        editor.material_panel->render(window_flags, ecs, selected_entity_id);
        editor.properties_panel->render(window_flags, ecs, selected_entity_id);
        editor.settings_panel->render(window_flags);
        editor.camera_panel->render(window_flags, camera);
        editor.status_panel->render(window_flags);

        ImGui::EndDisabled();

        // @TODO: this feels like a bit of a hack. We keep the viewport with its regular color
        // by rendering after the ImGui::EndDisabled()
        editor.viewport_panel->render(window_flags, camera, ecs, selected_entity_id, viewport_image);

        // End
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd.get_handle());
    }
};  // namespace mag
