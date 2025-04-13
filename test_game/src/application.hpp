#pragma once

#include <magnolia.hpp>

namespace mag
{
    class RenderGraph;
};

namespace game
{
    using namespace mag::math;

    class TestGame : public mag::Application
    {
        public:
            TestGame(const str& config_file_path);
            ~TestGame();

            virtual void on_update(const f32 dt) override;
            virtual void on_event(const mag::Event& e) override;

        private:
            void build_render_graph(const uvec2& size);

            unique<mag::Scene> scene = nullptr;
            unique<mag::RenderGraph> render_graph = nullptr;
    };
};  // namespace game
