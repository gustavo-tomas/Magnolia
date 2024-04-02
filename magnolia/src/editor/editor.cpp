#include "editor/editor.hpp"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "core/logger.hpp"

namespace mag
{
    void Editor::initialize(Window &window)
    {
        this->window = std::addressof(window);

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

        io.Fonts->AddFontFromFileTTF("assets/fonts/Source_Code_Pro/static/SourceCodePro-Regular.ttf", 15);

        this->set_style();

        ASSERT(ImGui_ImplSDL2_InitForVulkan(window.get_handle()), "Failed to initialize editor window backend");

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

    void Editor::update(CommandBuffer &cmd, const Image &viewport_image, std::vector<Model> &models)
    {
        // @TODO: this is not very pretty
        if (image_descriptor == nullptr)
        {
            image_descriptor =
                ImGui_ImplVulkan_AddTexture(viewport_image.get_sampler().get_handle(), viewport_image.get_image_view(),
                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        // Transition the draw image into their correct transfer layouts
        cmd.transfer_layout(viewport_image.get_image(), vk::ImageLayout::eTransferSrcOptimal,
                            vk::ImageLayout::eShaderReadOnlyOptimal);

        // @TODO: put this inside the render pass?
        render_pass.before_render(cmd);
        cmd.begin_rendering(this->render_pass.get_pass());

        // Begin
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(window->get_handle());
        ImGui::NewFrame();

        const ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;
        const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;

        // ImGui windows goes here
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dock_flags);
        // ImGui::ShowDemoWindow();

        render_panel(window_flags);
        render_viewport(window_flags, viewport_image);
        render_properties(window_flags, models);

        // @TODO: rendering empty window just for symmetry. This will be changed in the future.
        render_dummy(window_flags, "Dummy");

        // End
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd.get_handle());

        cmd.end_rendering();
        render_pass.after_render(cmd);

        // Return the draw image to their original layout
        cmd.transfer_layout(viewport_image.get_image(), vk::ImageLayout::eShaderReadOnlyOptimal,
                            vk::ImageLayout::eTransferSrcOptimal);
    }

    void Editor::render_dummy(const ImGuiWindowFlags window_flags, const str &name)
    {
        ImGui::Begin(name.c_str(), NULL, window_flags);
        ImGui::Text("%s", name.c_str());
        ImGui::End();
    }

    void Editor::render_panel(const ImGuiWindowFlags window_flags)
    {
        ImGui::Begin("Panel", NULL, window_flags);
        ImGui::Text("Use WASD and CTRL/ESCAPE to navigate");
        ImGui::Text("Press ESC to enter fullscreen mode");
        ImGui::Text("Press TAB to capture the cursor");
        ImGui::Text("Press KEY_DOWN/KEY_UP to scale image resolution");
        ImGui::Text("Press SHIFT to alternate between editor and scene views");
        ImGui::End();
    }

    void Editor::render_viewport(const ImGuiWindowFlags window_flags, const Image &viewport_image)
    {
        ImGui::Begin("Viewport", NULL, window_flags);

        const ImVec2 image_size(viewport_image.get_extent().width, viewport_image.get_extent().height);
        const ImVec2 viewport_size = ImGui::GetContentRegionAvail();

        // See this: https://www.reddit.com/r/opengl/comments/114lxvr/comment/j91nuyz/

        // Calculate the aspect ratio of the image and the content region
        const f32 image_aspect_ratio = image_size.x / image_size.y;
        const f32 viewport_aspect_ratio = viewport_size.x / viewport_size.y;

        // Scale the image horizontally if the content region is wider than the image
        if (viewport_aspect_ratio > image_aspect_ratio)
        {
            const f32 image_width = viewport_size.y * image_aspect_ratio;
            const f32 offset = (viewport_size.x - image_width) / 2;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
            ImGui::Image(image_descriptor, ImVec2(image_width, viewport_size.y));
        }

        // Scale the image vertically if the content region is taller than the image
        else
        {
            const f32 image_height = viewport_size.x / image_aspect_ratio;
            const f32 offset = (viewport_size.y - image_height) / 2;
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
            ImGui::Image(image_descriptor, ImVec2(viewport_size.x, image_height));
        }

        ImGui::End();
    }

    void Editor::render_properties(const ImGuiWindowFlags window_flags, std::vector<Model> &models)
    {
        // @TODO: check imguizmo implementation
        ImGui::Begin("Properties", NULL, window_flags);

        for (auto &model : models)
        {
            if (ImGui::TreeNodeEx(model.name.c_str()))
            {
                vec3 position = model.position;
                vec3 rotation = model.rotation;
                vec3 scale = model.scale;

                ImGui::Text("Position");
                if (ImGui::InputFloat3("##Position", value_ptr(position)) && ImGui::IsKeyPressed(ImGuiKey_Enter))
                    model.position = position;

                ImGui::Text("Rotation");
                if (ImGui::InputFloat3("##Rotation", value_ptr(rotation)) && ImGui::IsKeyPressed(ImGuiKey_Enter))
                    model.rotation = rotation;

                ImGui::Text("Scale");
                if (ImGui::InputFloat3("##Scale", value_ptr(scale)) && ImGui::IsKeyPressed(ImGuiKey_Enter))
                    model.scale = scale;

                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

    void Editor::process_events(SDL_Event &e) { ImGui_ImplSDL2_ProcessEvent(&e); }

    void Editor::on_resize(const uvec2 &size) { this->render_pass.on_resize(size); }

    void Editor::set_style()
    {
        ImGuiStyle &style = ImGui::GetStyle();

        style.Alpha = 1.0f;
        style.WindowRounding = 3.0f;
        style.WindowBorderSize = 0.0f;
        style.FrameRounding = 3.0f;
        style.FrameBorderSize = 0.0f;
        style.FramePadding = ImVec2(6.0f, 4.0f);
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
