#pragma once

#include <memory>

#include "core/event.hpp"
#include "core/types.hpp"
#include "imgui.h"
#include "menu/info_menu.hpp"

namespace mag
{
    class MenuBar
    {
        public:
            MenuBar();
            ~MenuBar() = default;

            void render(const ImGuiWindowFlags window_flags);
            b8 quit_requested() const { return quit; };

            void on_event(Event& e);

        private:
            // @TODO: it might be better to handle dialogs natively, but its better to setup the windows build first
            enum DialogAction
            {
                None = 0,
                New,
                Save,
                Open
            };

            void display_dialog();
            void new_scene();
            void save_active_scene();
            void save_active_scene_as();
            void open_scene();
            void quit_application();

            void on_key_press(KeyPressEvent& e);

            void set_dialog_action(const DialogAction action) { current_action = action; };

            DialogAction current_action = DialogAction::None;

            std::unique_ptr<InfoMenu> info_menu;
            b8 quit = false;
            str scene_file_path = "";
    };
};  // namespace mag