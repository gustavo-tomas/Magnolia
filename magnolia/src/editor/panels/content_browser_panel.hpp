#pragma once

#include <memory>

#include "imgui.h"
#include "renderer/image.hpp"

namespace mag
{
    class ContentBrowserPanel
    {
        public:
            ContentBrowserPanel();
            ~ContentBrowserPanel() = default;

            void render(const ImGuiWindowFlags window_flags);

        private:
            std::shared_ptr<Image> asset_image;
            vk::DescriptorSet asset_image_descriptor = {};
    };
};  // namespace mag
