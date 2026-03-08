#pragma once

#include <magnolia/core/types.hpp>

namespace mag
{
    struct Event;
    struct QuitEvent;
    struct WindowCloseEvent;
    class Project;
};  // namespace mag

namespace game
{
    class Renderer;
    class Scene;

    class TestGame
    {
        public:
            TestGame();
            ~TestGame();

            void run();

        private:
            void on_update(const f32 dt);
            void on_event(const mag::Event& e);

            void register_commands();

            void on_window_close(const mag::WindowCloseEvent& e);
            void on_quit(const mag::QuitEvent& e);

            b8 running = false;

            mag::unique<Renderer> renderer;
            mag::unique<Scene> scene;
            mag::unique<mag::Project> project;
    };
};  // namespace game
