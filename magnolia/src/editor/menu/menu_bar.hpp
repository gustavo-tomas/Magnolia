#pragma once

#include <memory>

#include "editor/menu/info_menu.hpp"
#include "imgui.h"

namespace mag
{
    class MenuBar
    {
        public:
            MenuBar();
            ~MenuBar() = default;

            void render(const ImGuiWindowFlags window_flags);

        private:
            std::unique_ptr<InfoMenu> info_menu;
    };
};  // namespace mag
