#include "editor.hpp"

#include <core/entry_point.hpp>
#include <vulkan/vulkan.hpp>

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "editor_scene.hpp"
#include "editor_style.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "implot/implot.h"
#include "math/generic.hpp"
#include "menu/menu_bar.hpp"
#include "panels/camera_panel.hpp"
#include "panels/content_browser_panel.hpp"
#include "panels/materials_panel.hpp"
#include "panels/properties_panel.hpp"
#include "panels/scene_panel.hpp"
#include "panels/settings_panel.hpp"
#include "panels/status_panel.hpp"
#include "panels/viewport_panel.hpp"
#include "passes/editor_pass.hpp"
#include "passes/scene_pass.hpp"
#include "physics/physics.hpp"
#include "renderer/context.hpp"
#include "renderer/render_graph.hpp"
#include "renderer/renderer.hpp"
#include "scene/scene_serializer.hpp"

mag::Application *mag::create_application() { return new sprout::Editor("sprout_editor/config.json"); }

namespace sprout
{
    Editor &get_editor() { return static_cast<Editor &>(get_application()); }

    struct Editor::IMPL
    {
            IMPL() = default;
            ~IMPL() = default;

            vk::DescriptorPool descriptor_pool;

            unique<MenuBar> menu_bar;
            unique<ContentBrowserPanel> content_browser_panel;
            unique<ViewportPanel> viewport_panel;
            unique<ScenePanel> scene_panel;
            unique<MaterialsPanel> material_panel;
            unique<PropertiesPanel> properties_panel;
            unique<StatusPanel> status_panel;
            unique<CameraPanel> camera_panel;
            unique<SettingsPanel> settings_panel;

            unique<RenderGraph> render_graph;
            std::vector<ref<EditorScene>> open_scenes;
            std::vector<u32> open_scenes_marked_for_deletion;

            u32 selected_scene_index = 0;
            u32 next_scene_index = 0;

            uvec2 curr_viewport_size;
            b8 disabled = false;
    };

    Editor::Editor(const str &config_file_path) : Application(config_file_path), impl(new IMPL())
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

        impl->descriptor_pool = context.get_device().createDescriptorPool(create_info);

        // Initialize imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();

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

        const str icon_path = "sprout_editor/assets/fonts/FontAwesome/" FONT_ICON_FILE_NAME_FAS;
        const str font_path = "sprout_editor/assets/fonts/Source_Code_Pro/static/SourceCodePro-Regular.ttf";

        io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_size);
        io.Fonts->AddFontFromFileTTF(icon_path.c_str(), icon_font_size, &icons_config, icons_ranges);

        set_default_editor_style(ImGui::GetStyle());

        ASSERT(ImGui_ImplSDL2_InitForVulkan(static_cast<SDL_Window *>(get_application().get_window().get_handle())),
               "Failed to initialize editor window backend");

        const vk::Format color_attachment_format = context.get_supported_color_format(ImageFormat::Float);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = context.get_instance();
        init_info.PhysicalDevice = context.get_physical_device();
        init_info.Device = device;
        init_info.Queue = context.get_graphics_queue();
        init_info.DescriptorPool = static_cast<VkDescriptorPool>(impl->descriptor_pool);
        init_info.MinImageCount = context.get_swapchain_images().size();
        init_info.ImageCount = context.get_swapchain_images().size();
        init_info.UseDynamicRendering = true;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
        init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats =
            reinterpret_cast<const VkFormat *>(&color_attachment_format);

        ASSERT(ImGui_ImplVulkan_Init(&init_info), "Failed to initialize editor renderer backend");

        ASSERT(ImGui_ImplVulkan_CreateFontsTexture(), "Failed to create editor fonts texture");

        impl->menu_bar = create_unique<MenuBar>();
        impl->content_browser_panel = create_unique<ContentBrowserPanel>();
        impl->viewport_panel = create_unique<ViewportPanel>();
        impl->scene_panel = create_unique<ScenePanel>();
        impl->material_panel = create_unique<MaterialsPanel>();
        impl->status_panel = create_unique<StatusPanel>();
        impl->camera_panel = create_unique<CameraPanel>();
        impl->settings_panel = create_unique<SettingsPanel>();

        // Initialize render graph
        auto &app = get_application();
        auto &window = app.get_window();

        const uvec2 window_size = window.get_size();
        impl->curr_viewport_size = window_size;

        build_render_graph(window_size, get_viewport_size());

        EditorScene *scene = new EditorScene();
        SceneSerializer scene_serializer(*scene);
        scene_serializer.deserialize("sprout_editor/assets/scenes/Test.mag.json");

        add_scene(scene);
        set_active_scene(0);
    }

    Editor::~Editor()
    {
        auto &context = get_context();

        ImGui_ImplVulkan_DestroyFontsTexture();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        context.get_device().destroyDescriptorPool(impl->descriptor_pool);
    }

    void Editor::on_update(const f32 dt)
    {
        (void)dt;

        auto &app = get_application();
        auto &window = app.get_window();
        auto &renderer = app.get_renderer();

        // Delete closed scenes from back to front
        for (i32 i = impl->open_scenes_marked_for_deletion.size() - 1; i >= 0; i--)
        {
            const u32 pos = impl->open_scenes_marked_for_deletion[i];

            impl->open_scenes.erase(impl->open_scenes.begin() + pos);
            impl->open_scenes_marked_for_deletion.erase(impl->open_scenes_marked_for_deletion.begin() + i);

            if (pos < impl->selected_scene_index)
            {
                impl->selected_scene_index =
                    math::clamp(impl->selected_scene_index - 1, 0u, static_cast<u32>(impl->open_scenes.size() - 1));
            }

            else if (pos == impl->selected_scene_index)
            {
                impl->selected_scene_index =
                    math::clamp(impl->selected_scene_index - 1, 0u, static_cast<u32>(impl->open_scenes.size() - 1));

                set_active_scene(impl->selected_scene_index);
            }
        }

        if (impl->selected_scene_index != impl->next_scene_index)
        {
            set_active_scene(impl->next_scene_index);
        }

        auto &active_scene = get_active_scene();

        // @TODO: this is a hack. We want to resize the editor viewport but we want to decouple the application from the
        // rest of the engine. We should find a better solution to handle application side events.
        const auto &viewport_size = impl->viewport_panel->get_viewport_size();
        active_scene.on_viewport_resize(viewport_size);
        this->on_viewport_resize(viewport_size);

        active_scene.on_update(dt);

        if (impl->menu_bar->quit_requested())
        {
            auto event = QuitEvent();
            process_user_application_event(event);
        }

        // Alternate between Fullscreen and Windowed
        if (window.is_key_pressed(Key::Escape))
        {
            window.set_fullscreen(!window.is_fullscreen());
        }

        renderer.on_update(get_render_graph());
    }

    void Editor::render(ECS &ecs, Camera &camera, RendererImage &viewport_image)
    {
        // Disable widgets
        ImGui::BeginDisabled(impl->disabled);

        const ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode |
                                              static_cast<ImGuiDockNodeFlags>(ImGuiDockNodeFlags_NoWindowMenuButton);

        const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;

        // ImGui windows goes here
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dock_flags);
        // ImGui::ShowDemoWindow();

        impl->scene_panel->render(window_flags, ecs);
        const u64 selected_entity_id = impl->scene_panel->get_selected_entity_id();

        impl->menu_bar->render(window_flags);
        impl->content_browser_panel->render(window_flags);
        impl->material_panel->render(window_flags, ecs, selected_entity_id);
        impl->properties_panel->render(window_flags, ecs, selected_entity_id);
        impl->settings_panel->render(window_flags);
        impl->camera_panel->render(window_flags, camera);
        impl->status_panel->render(window_flags);

        ImGui::EndDisabled();

        // @TODO: this feels like a bit of a hack. We keep the viewport with its regular color
        // by rendering after the ImGui::EndDisabled()
        impl->viewport_panel->render(window_flags, camera, ecs, selected_entity_id, viewport_image);
    }

    void Editor::on_event(Event &e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<NativeEvent>(BIND_FN(Editor::on_sdl_event));
        dispatcher.dispatch<WindowResizeEvent>(BIND_FN(Editor::on_resize));
        dispatcher.dispatch<QuitEvent>(BIND_FN(Editor::on_quit));

        get_active_scene().on_event(e);
        impl->menu_bar->on_event(e);
        impl->viewport_panel->on_event(e);
    }

    // @TODO: use file dialogs to ask about saving files when the file dialog is implemented. For now this is a
    // temporary solution to avoid losing work after closing. Also the scene names must be different form one another or
    // else the files will be overwritten.
    void Editor::on_quit(QuitEvent &e)
    {
        (void)e;

        // Save open scenes
        for (const auto &scene : impl->open_scenes)
        {
            close_scene(scene);
        }
    }

    void Editor::on_sdl_event(NativeEvent &e) { ImGui_ImplSDL2_ProcessEvent(reinterpret_cast<const SDL_Event *>(e.e)); }

    void Editor::on_resize(WindowResizeEvent &e)
    {
        const uvec2 window_size = {e.width, e.height};

        build_render_graph(window_size, get_viewport_size());
    }

    void Editor::on_viewport_resize(const uvec2 &new_viewport_size)
    {
        if (new_viewport_size == impl->curr_viewport_size) return;

        impl->curr_viewport_size = new_viewport_size;

        const uvec2 window_size = get_application().get_window().get_size();

        build_render_graph(window_size, new_viewport_size);
    }

    void Editor::add_scene(EditorScene *scene) { impl->open_scenes.emplace_back(scene); }

    void Editor::close_scene(const ref<EditorScene> &scene)
    {
        const str file_path = "sprout_editor/assets/scenes/" + scene->get_name() + ".mag.json";

        SceneSerializer serializer(*scene);
        serializer.serialize(file_path);

        // @TODO: linear search but w e. It is unlikely that this will hinder the performance
        u32 idx = INVALID_ID;
        for (u32 i = 0; i < impl->open_scenes.size(); i++)
        {
            if (impl->open_scenes[i] == scene)
            {
                idx = i;
                impl->open_scenes_marked_for_deletion.push_back(idx);
                break;
            }
        }

        // Sort to avoid problems during deletion
        std::sort(impl->open_scenes_marked_for_deletion.begin(), impl->open_scenes_marked_for_deletion.end());
    }

    void Editor::set_active_scene(const u32 index)
    {
        auto &app = get_application();
        auto &physics_engine = app.get_physics_engine();

        auto &active_scene = get_active_scene();
        if (active_scene.is_running())
        {
            active_scene.on_stop();
        }

        impl->selected_scene_index = math::clamp(index, 0u, static_cast<u32>(impl->open_scenes.size() - 1));

        physics_engine.on_simulation_start(impl->open_scenes[impl->selected_scene_index].get());
    }

    void Editor::set_input_disabled(const b8 disable) { impl->disabled = disable; }

    void Editor::set_selected_scene_index(const u32 index)
    {
        impl->next_scene_index = math::clamp(index, 0u, static_cast<u32>(impl->open_scenes.size() - 1));
    }

    b8 Editor::is_viewport_window_active() const { return impl->viewport_panel->is_viewport_window_active(); }

    void Editor::build_render_graph(const uvec2 &size, const uvec2 &viewport_size)
    {
        impl->render_graph.reset(new RenderGraph());

        // @TODO: for now only one output attachment of each type is supported (one color and one depth maximum)

        DepthPrePass *depth_prepass = new DepthPrePass(viewport_size);
        ScenePass *scene_pass = new ScenePass(viewport_size);
        GizmoPass *gizmo_pass = new GizmoPass(viewport_size);
        EditorPass *editor_pass = new EditorPass(size);

        impl->render_graph->set_output_attachment("EditorOutputColor");
        // impl->render_graph->set_output_attachment("OutputColor");

        impl->render_graph->add_pass(depth_prepass);
        impl->render_graph->add_pass(scene_pass);
        impl->render_graph->add_pass(gizmo_pass);
        impl->render_graph->add_pass(editor_pass);

        impl->render_graph->build();
    }

    EditorScene &Editor::get_active_scene() { return *impl->open_scenes[impl->selected_scene_index]; }
    RenderGraph &Editor::get_render_graph() { return *impl->render_graph; }
    const std::vector<ref<EditorScene>> &Editor::get_open_scenes() const { return impl->open_scenes; }
    u32 Editor::get_selected_scene_index() const { return impl->selected_scene_index; }
    const uvec2 &Editor::get_viewport_size() const { return impl->viewport_panel->get_viewport_size(); }

    // @TODO: find a better way to pass values to the rest of the application (maybe use a struct?)
    u32 &Editor::get_texture_output() { return impl->settings_panel->get_texture_output(); }
    u32 &Editor::get_normal_output() { return impl->settings_panel->get_normal_output(); }
    b8 &Editor::is_bounding_box_enabled() { return impl->settings_panel->is_bounding_box_enabled(); }
    b8 &Editor::is_physics_colliders_enabled() { return impl->settings_panel->is_physics_colliders_enabled(); }
    b8 &Editor::is_gizmos_enabled() { return impl->settings_panel->is_gizmos_enabled(); }
    b8 Editor::is_disabled() const { return impl->disabled; }
};  // namespace sprout
