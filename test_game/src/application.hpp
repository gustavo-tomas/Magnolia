#pragma once

#include <magnolia.hpp>

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

            void render_sprites();
            void render_models();
            void render_text();

        private:
            unique<mag::Scene> scene = nullptr;
            mag::gfx::ShaderHandle sprite_shader;
            mag::gfx::ShaderHandle mesh_shader;
            mag::gfx::ShaderHandle text_shader;
    };
};  // namespace game
