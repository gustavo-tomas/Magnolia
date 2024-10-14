#include "editor.hpp"

#include <core/entry_point.hpp>

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "editor_style.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui.h"
#include "implot/implot.h"
#include "passes/editor_pass.hpp"
#include "passes/scene_pass.hpp"
#include "scene/scene_serializer.hpp"

mag::Application *mag::create_application()
{
    mag::ApplicationOptions options;
    options.title = "Sprout";
    options.window_icon = "sprout_editor/assets/images/application_icon.bmp";

    return new sprout::Editor(options);
}

namespace sprout
{
    Editor &get_editor() { return static_cast<Editor &>(get_application()); }

    Editor::Editor(const ApplicationOptions &options) : Application(options)
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

        menu_bar = create_unique<MenuBar>();
        content_browser_panel = create_unique<ContentBrowserPanel>();
        viewport_panel = create_unique<ViewportPanel>();
        scene_panel = create_unique<ScenePanel>();
        material_panel = create_unique<MaterialsPanel>();
        status_panel = create_unique<StatusPanel>();
        camera_panel = create_unique<CameraPanel>();
        settings_panel = create_unique<SettingsPanel>();

        // Initialize render graph
        auto &app = get_application();
        auto &window = app.get_window();

        const uvec2 window_size = window.get_size();
        curr_viewport_size = window_size;

        build_render_graph(window_size, get_viewport_size());

        Scene *scene = new Scene();
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

        context.get_device().destroyDescriptorPool(descriptor_pool);
    }

    void Editor::on_update(const f32 dt)
    {
        (void)dt;

        auto &app = get_application();
        auto &window = app.get_window();
        auto &renderer = app.get_renderer();

        // Delete closed scenes from back to front
        for (i32 i = open_scenes_marked_for_deletion.size() - 1; i >= 0; i--)
        {
            const u32 pos = open_scenes_marked_for_deletion[i];

            open_scenes.erase(open_scenes.begin() + pos);
            open_scenes_marked_for_deletion.erase(open_scenes_marked_for_deletion.begin() + i);

            if (pos < selected_scene_index)
            {
                selected_scene_index =
                    math::clamp(selected_scene_index - 1, 0u, static_cast<u32>(open_scenes.size() - 1));
            }

            else if (pos == selected_scene_index)
            {
                selected_scene_index =
                    math::clamp(selected_scene_index - 1, 0u, static_cast<u32>(open_scenes.size() - 1));

                set_active_scene(selected_scene_index);
            }
        }

        if (selected_scene_index != next_scene_index)
        {
            set_active_scene(next_scene_index);
        }

        auto &active_scene = get_active_scene();

        // @TODO: this is a hack. We want to resize the editor viewport but we want to decouple the application from the
        // rest of the engine. We should find a better solution to handle application side events.
        const auto &viewport_size = viewport_panel->get_viewport_size();
        active_scene.on_viewport_resize(viewport_size);
        this->on_viewport_resize(viewport_size);

        active_scene.on_update(dt);

        if (menu_bar->quit_requested())
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

    void Editor::on_event(Event &e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<NativeEvent>(BIND_FN(Editor::on_sdl_event));
        dispatcher.dispatch<WindowResizeEvent>(BIND_FN(Editor::on_resize));
        dispatcher.dispatch<QuitEvent>(BIND_FN(Editor::on_quit));

        get_active_scene().on_event(e);
        menu_bar->on_event(e);
        viewport_panel->on_event(e);
    }

    // @TODO: use file dialogs to ask about saving files when the file dialog is implemented. For now this is a
    // temporary solution to avoid losing work after closing. Also the scene names must be different form one another or
    // else the files will be overwritten.
    void Editor::on_quit(QuitEvent &e)
    {
        (void)e;

        // Save open scenes
        for (const auto &scene : open_scenes)
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
        if (new_viewport_size == curr_viewport_size) return;

        curr_viewport_size = new_viewport_size;

        const uvec2 window_size = get_application().get_window().get_size();

        build_render_graph(window_size, new_viewport_size);
    }

    void Editor::add_scene(Scene *scene) { open_scenes.emplace_back(scene); }

    void Editor::close_scene(const ref<Scene> &scene)
    {
        const str file_path = "sprout_editor/assets/scenes/" + scene->get_name() + ".mag.json";

        SceneSerializer serializer(*scene);
        serializer.serialize(file_path);

        // @TODO: linear search but w e. It is unlikely that this will hinder the performance
        u32 idx = INVALID_ID;
        for (u32 i = 0; i < open_scenes.size(); i++)
        {
            if (open_scenes[i] == scene)
            {
                idx = i;
                open_scenes_marked_for_deletion.push_back(idx);
                break;
            }
        }

        // Sort to avoid problems during deletion
        std::sort(open_scenes_marked_for_deletion.begin(), open_scenes_marked_for_deletion.end());
    }

    void Editor::set_active_scene(const u32 index)
    {
        auto &active_scene = get_active_scene();
        if (active_scene.get_scene_state() == SceneState::Runtime)
        {
            get_active_scene().stop_runtime();
        }

        selected_scene_index = math::clamp(index, 0u, static_cast<u32>(open_scenes.size() - 1));

        auto &app = get_application();
        auto &physics_engine = app.get_physics_engine();

        physics_engine.on_simulation_start(open_scenes[selected_scene_index].get());
    }

    void Editor::set_input_disabled(const b8 disable) { this->disabled = disable; }

    void Editor::set_selected_scene_index(const u32 index)
    {
        next_scene_index = math::clamp(index, 0u, static_cast<u32>(open_scenes.size() - 1));
    }

    b8 Editor::is_viewport_window_active() const { return viewport_panel->is_viewport_window_active(); }

    void Editor::build_render_graph(const uvec2 &size, const uvec2 &viewport_size)
    {
        render_graph.reset(new RenderGraph());

        // @TODO: for now only one output attachment of each type is supported (one color and one depth maximum)

        ScenePass *scene_pass = new ScenePass(viewport_size);
        GizmoPass *gizmo_pass = new GizmoPass(viewport_size);
        EditorPass *editor_pass = new EditorPass(size);

        render_graph->set_output_attachment("EditorOutputColor");
        // render_graph->set_output_attachment("OutputColor");

        render_graph->add_pass(scene_pass);
        render_graph->add_pass(gizmo_pass);
        render_graph->add_pass(editor_pass);

        render_graph->build();
    }
};  // namespace sprout
