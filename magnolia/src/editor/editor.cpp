#include "editor/editor.hpp"

#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

namespace mag
{
    void Editor::initialize(Window &window)
    {
        this->window = std::addressof(window);

        // Context creation
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto &io = ImGui::GetIO();
        (void)io;
    }

    void Editor::shutdown() { ImGui::DestroyContext(); }

    void Editor::update() {}
};  // namespace mag