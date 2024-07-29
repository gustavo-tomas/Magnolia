#pragma once

#include <memory>
#include <vector>

#include "camera/camera.hpp"
#include "ecs/ecs.hpp"
#include "renderer/image.hpp"
#include "renderer/model.hpp"
#include "renderer/shader.hpp"

// @TODO: review this whole chicanery

namespace mag
{
    using namespace mag::math;

    class CommandBuffer;

    struct Pass
    {
            vk::RenderingInfo rendering_info;
            vk::RenderingAttachmentInfo color_attachment;
            vk::RenderingAttachmentInfo depth_attachment;

            Statistics statistics = {};
    };

    class StandardRenderPass
    {
        public:
            StandardRenderPass(const uvec2& size);
            ~StandardRenderPass();

            void before_render();
            void render(const Camera& camera, ECS& ecs);
            void after_render();

            void set_render_scale(const f32 scale);
            void on_resize(const uvec2& size);

            Pass& get_pass() { return pass; };
            vec4& get_clear_color() { return clear_color; };
            const Image& get_target_image() const;
            const uvec3& get_draw_size() const { return draw_size; };
            f32 get_render_scale() const { return render_scale; };

        private:
            void initialize_images();

            Pass pass = {};
            std::shared_ptr<Shader> mesh_shader, grid_shader, physics_line_shader;
            std::vector<Image> draw_images, depth_images, resolve_images;
            std::unique_ptr<Line> physics_debug_lines;

            uvec3 draw_size;
            f32 render_scale = 1.0;
            vec4 clear_color = vec4(0.1f, 0.1f, 0.1f, 1.0f);
    };
};  // namespace mag
