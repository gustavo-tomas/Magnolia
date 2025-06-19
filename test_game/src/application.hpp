#pragma once

#include <magnolia/core/application.hpp>
#include <magnolia/scene/scene.hpp>

namespace game
{
    using namespace mag::math;

    class Renderer;

    class TestGame : public mag::Application
    {
        public:
            TestGame(const str& config_file_path);
            ~TestGame();

            virtual void on_update(const f32 dt) override;
            virtual void on_event(const mag::Event& e) override;

        private:
            void on_resource_loaded(const mag::IResource* resource);

            unique<Renderer> renderer = nullptr;
            unique<mag::Scene> scene = nullptr;
    };
};  // namespace game
