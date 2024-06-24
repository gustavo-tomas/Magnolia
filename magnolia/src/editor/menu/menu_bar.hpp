#pragma once

#include <memory>

#include "core/types.hpp"
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
            b8 quit_requested() const { return quit; };

        private:
            void display_dialog();

            std::unique_ptr<InfoMenu> info_menu;
            b8 quit = false;
            b8 dialog_open = false;
    };
};  // namespace mag
