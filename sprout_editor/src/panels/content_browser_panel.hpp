#pragma once

#include <filesystem>

#include "imgui.h"
#include "renderer/renderer_image.hpp"

namespace sprout
{
    using namespace mag;

    class ContentBrowserPanel
    {
        public:
            ContentBrowserPanel(const std::filesystem::path& base_directory);
            ~ContentBrowserPanel() = default;

            void render(const ImGuiWindowFlags window_flags);

        private:
            std::filesystem::path base_directory;
            std::filesystem::path current_directory;

            ref<RendererImage> folder_image, file_image;
            vk::DescriptorSet folder_image_descriptor = {}, file_image_descriptor = {};
    };
};  // namespace sprout
