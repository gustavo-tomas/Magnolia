#include "editor.hpp"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "core/application.hpp"
#include "core/assert.hpp"
#include "editor_style.hpp"
#include "icon_font_cpp/IconsFontAwesome6.h"
#include "imgui.h"
#include "passes/editor_pass.hpp"
#include "passes/scene_pass.hpp"
#include "scene/scene_serializer.hpp"

namespace sprout
{
    static Editor *editor = nullptr;

    Editor &get_editor()
    {
        ASSERT(editor != nullptr, "Editor is null");
        return *editor;
    }

    Editor::Editor(const EventCallback &event_callback) : event_callback(event_callback)
    {
        editor = this;

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

        menu_bar = std::make_unique<MenuBar>();
        content_browser_panel = std::make_unique<ContentBrowserPanel>();
        viewport_panel = std::make_unique<ViewportPanel>();
        scene_panel = std::make_unique<ScenePanel>();
        material_panel = std::make_unique<MaterialsPanel>();
        status_panel = std::make_unique<StatusPanel>();
        camera_panel = std::make_unique<CameraPanel>();
        settings_panel = std::make_unique<SettingsPanel>();

        // Initialize render graph
        auto &app = get_application();
        auto &window = app.get_window();

        const uvec2 window_size = window.get_size();
        curr_viewport_size = window_size;

        build_render_graph(window_size, get_viewport_size());
    }

    Editor::~Editor()
    {
        auto &context = get_context();

        active_scene.reset();

        ImGui_ImplVulkan_DestroyFontsTexture();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        context.get_device().destroyDescriptorPool(descriptor_pool);
    }

    void Editor::on_attach()
    {
        Scene *scene = new Scene();
        SceneSerializer scene_serializer(*scene);
        scene_serializer.deserialize("sprout_editor/assets/scenes/test_scene.mag.json");

        set_active_scene(scene);
    }

    void Editor::on_update(const f32 dt)
    {
        (void)dt;

        auto &app = get_application();
        auto &window = app.get_window();
        auto &renderer = app.get_renderer();
        auto &physics_engine = app.get_physics_engine();

        if (!scene_queue.empty())
        {
            set_active_scene(scene_queue.front());
            scene_queue.clear();
        }

        // @TODO: this is a hack. We want to resize the editor viewport but we want to decouple the application from the
        // rest of the engine. We should find a better solution to handle application side events.
        const auto &viewport_size = viewport_panel->get_viewport_size();
        active_scene->on_viewport_resize(viewport_size);
        this->on_viewport_resize(viewport_size);

        physics_engine.update(dt);

        active_scene->update(dt);

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

        renderer.update(get_render_graph());
    }

    void Editor::on_event(Event &e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<NativeEvent>(BIND_FN(Editor::on_sdl_event));
        dispatcher.dispatch<WindowResizeEvent>(BIND_FN(Editor::on_resize));

        active_scene->on_event(e);
        menu_bar->on_event(e);
        viewport_panel->on_event(e);
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

    void Editor::enqueue_scene(Scene *scene) { scene_queue.push_back(scene); }

    void Editor::set_active_scene(Scene *scene)
    {
        auto &app = get_application();
        auto &physics_engine = app.get_physics_engine();

        active_scene = std::unique_ptr<Scene>(scene);
        physics_engine.on_simulation_start(scene);
    }

    void Editor::set_input_disabled(const b8 disable) { this->disabled = disable; }

    b8 Editor::is_viewport_window_active() const { return viewport_panel->is_viewport_window_active(); }

    void Editor::build_render_graph(const uvec2 &size, const uvec2 &viewport_size)
    {
        render_graph.reset(new RenderGraph());

        // @TODO: for now only one output attachment of each type is supported (one color and one depth maximum)

        ScenePass *scene_pass = new ScenePass(viewport_size);
        LinePass *line_pass = new LinePass(viewport_size);
        GridPass *grid_pass = new GridPass(viewport_size);
        EditorPass *editor_pass = new EditorPass(size);

        render_graph->set_output_attachment("EditorOutputColor");
        // render_graph->set_output_attachment("OutputColor");

        render_graph->add_pass(scene_pass);
        render_graph->add_pass(line_pass);
        render_graph->add_pass(grid_pass);
        render_graph->add_pass(editor_pass);

        render_graph->build();
    }
};  // namespace sprout
