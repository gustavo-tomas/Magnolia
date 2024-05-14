#pragma once

#include <memory>
#include <vector>

#include "camera/camera.hpp"
#include "ecs/ecs.hpp"
#include "renderer/descriptors.hpp"
#include "renderer/image.hpp"
#include "renderer/model.hpp"
#include "renderer/pipeline.hpp"

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
    };

    class StandardRenderPass
    {
        public:
            StandardRenderPass(const uvec2& size);
            ~StandardRenderPass();

            void before_render(CommandBuffer& command_buffer);
            void render(CommandBuffer& command_buffer, const Camera& camera, ECS& ecs);
            void after_render(CommandBuffer& command_buffer);

            // @TODO: temp
            void add_model(const Model& model);

            Pass& get_pass() { return pass; };
            vec4& get_clear_color() { return clear_color; };
            const Image& get_target_image() const;
            const uvec3& get_draw_size() const { return draw_size; };
            f32 get_render_scale() const { return render_scale; };

            void set_render_scale(const f32 scale);

            void on_resize(const uvec2& size);

        private:
            void initialize_images();
            void add_uniform_data(const u64 buffer_size);
            void add_uniform_texture(const Model& model);

            Pass pass = {};
            std::unique_ptr<Pipeline> triangle_pipeline, grid_pipeline;
            std::shared_ptr<Shader> triangle, grid;
            std::vector<Image> draw_images, depth_images, resolve_images;
            uvec3 draw_size;
            f32 render_scale = 1.0;
            vec4 clear_color = vec4(0.1f, 0.1f, 0.1f, 1.0f);

            // @TODO: temporary
            struct GlobalData
            {
                    // Camera
                    mat4 view;        // 64 bytes (16 x 4)
                    mat4 projection;  // 64 bytes (16 x 4)
                    vec2 near_far;    // 8  bytes (2  x 4)

                    // Padding <:( - 8  bytes (2  x 4)
                    vec2 gamer_padding_dont_use_this_is_just_for_padding_gamer_gaming_game;

                    // Light
                    vec4 point_light_color_and_intensity;  // 16 bytes (4 x 4)
                    vec3 point_light_position;             // 12 bytes (3 x 4)
            };

            struct InstanceData
            {
                    mat4 model;  // 64 bytes (16 x 4)
            };

            std::vector<std::vector<Buffer>> data_buffers;
            std::vector<Image> textures;
            std::vector<Descriptor> uniform_descriptors, image_descriptors;
            // @TODO: temporary
    };
};  // namespace mag
