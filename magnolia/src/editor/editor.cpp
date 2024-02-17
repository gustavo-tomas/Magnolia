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

        ASSERT(ImGui_ImplSDL2_InitForVulkan(window.get_handle()), "Failed to initialize editor window backend");

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = context.get_instance();
        init_info.PhysicalDevice = context.get_physical_device();
        init_info.Device = device;
        init_info.Queue = context.get_graphics_queue();
        init_info.DescriptorPool = static_cast<VkDescriptorPool>(descriptor_pool);
        init_info.MinImageCount = 3;
        init_info.ImageCount = 3;
        init_info.UseDynamicRendering = false;
        init_info.ColorAttachmentFormat = static_cast<VkFormat>(context.get_swapchain_image_format());
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ASSERT(ImGui_ImplVulkan_Init(&init_info, this->render_pass.get_pass().render_pass),
               "Failed to initialize editor renderer backend");

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

    void traverse_children(Node *node)
    {
        if (node == nullptr) return;

        ImGuiTreeNodeFlags flags = 0;
        if (node->get_children().empty()) flags = ImGuiTreeNodeFlags_Leaf;

        if (ImGui::TreeNodeEx(node->get_name().c_str(), flags))
        {
            for (const auto &child : node->get_children()) traverse_children(child);
            ImGui::TreePop();
        }
    }

    void Editor::update(Node *tree, CommandBuffer &cmd, const Image &viewport_image)
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
        render_pass.before_pass(cmd);
        cmd.begin_pass(this->render_pass.get_pass());

        // Begin
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(window->get_handle());
        ImGui::NewFrame();

        // ImGui windows goes here
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        // ImGui::ShowDemoWindow();

        // Info panel
        {
            ImGui::Begin("Panel");
            ImGui::Text("Use WASD and CTRL/ESCAPE to navigate");
            ImGui::Text("Press MOUSE WHEEL to rotate the camera");
            ImGui::Text("Press ESC to enter fullscreen mode");
            ImGui::Text("Press TAB to capture the cursor");
            ImGui::Text("Press KEY_DOWN/KEY_UP to scale image resolution");
            ImGui::Text("Press SHIFT to alternate between editor and scene views");
            ImGui::Checkbox("Fit image to viewport dimensions", &fit_inside_viewport);
            ImGui::End();
        }

        // Viewport
        {
            ImGui::Begin("Viewport");
            ImVec2 image_size(viewport_image.get_extent().width, viewport_image.get_extent().height);

            const ImVec2 window_size = ImGui::GetWindowSize();
            const f32 top_offset = 20.0f;

            if (fit_inside_viewport)
            {
                // Keep the entire image inside the viewport
                const f32 diff = window_size.y / image_size.y;
                image_size.x *= diff;
                image_size.y *= diff;

                // Center the image inside the window
                const ImVec2 image_position((window_size.x - image_size.x) * 0.5f,
                                            (window_size.y - image_size.y) * 0.5f + top_offset);
                ImGui::SetCursorPos(image_position);
            }

            else
            {
                // Keep the image static
                const ImVec2 image_position((window_size.x - image_size.x) * 0.5f, top_offset);
                ImGui::SetCursorPos(image_position);
            }

            ImGui::Image(image_descriptor, image_size);
            ImGui::End();
        }

        // Tree viewer
        {
            ImGui::Begin("Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::TreeNode("Tree Nodes"))
            {
                traverse_children(tree);
                ImGui::TreePop();
            }

            ImGui::End();
        }

        // End
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd.get_handle());

        cmd.end_pass(this->render_pass.get_pass());
        render_pass.after_pass(cmd);

        // Return the draw image to their original layout
        cmd.transfer_layout(viewport_image.get_image(), vk::ImageLayout::eShaderReadOnlyOptimal,
                            vk::ImageLayout::eTransferSrcOptimal);
    }

    void Editor::process_events(SDL_Event &e) { ImGui_ImplSDL2_ProcessEvent(&e); }

    void Editor::on_resize(const uvec2 &size) { this->render_pass.on_resize(size); }
};  // namespace mag