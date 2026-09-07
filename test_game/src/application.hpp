#pragma once

#include <magnolia/core/types.hpp>

#include "renderer.hpp"

namespace mag
{
    struct Event;
    struct QuitEvent;
    struct WindowCloseEvent;
    class Project;
};  // namespace mag

namespace game
{
    class Scene;

    class TestGame
    {
        public:
            TestGame();
            ~TestGame();

            void run();

        private:
            void on_update(f32 dt);
            void on_event(const mag::Event& e);

            void register_commands();

            void on_window_close(const mag::WindowCloseEvent& e);
            void on_quit(const mag::QuitEvent& e);

            Renderer renderer;
            mag::unique<Scene> scene;
            mag::unique<mag::Project> project;

            b8 running = false;
    };
};  // namespace game
