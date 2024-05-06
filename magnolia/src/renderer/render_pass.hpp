#pragma once

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
            void initialize(const uvec2& size);
            void shutdown();

            void before_render(CommandBuffer& command_buffer);
            void render(CommandBuffer& command_buffer, const Camera& camera, ECS& ecs);
            void after_render(CommandBuffer& command_buffer);

            // @TODO: temp
            void add_model(const Model& model);
            void set_camera();

            Pass& get_pass();
            const Image& get_target_image() const;
            f32 get_render_scale() const { return render_scale; };
            uvec3 get_draw_size() const { return draw_size; };
            void set_render_scale(const f32 scale);

            void on_resize(const uvec2& size);

        private:
            void initialize_images();
            void add_uniform_data(const u64 buffer_size);
            void add_uniform_texture(const Model& model);

            std::vector<Pass> passes = {};
            Pipeline triangle_pipeline, grid_pipeline;
            Shader triangle_vs, triangle_fs, grid_vs, grid_fs;
            std::vector<Image> draw_images, depth_images, resolve_images;
            uvec3 draw_size;
            f32 render_scale = 1.0;
            vk::PipelineBindPoint pipeline_bind_point;
            vk::Rect2D render_area;

            // @TODO: temporary
            struct CameraData
            {
                    mat4 view;        // 64 bytes (16 x 4)
                    mat4 projection;  // 64 bytes (16 x 4)
                    vec2 near_far;    // 8  bytes (2  x 4)
            };

            struct ModelData
            {
                    mat4 model;  // 64 bytes (16 x 4)
            };

            std::vector<std::vector<Buffer>> data_buffers;
            std::vector<Image> textures;
            std::vector<Descriptor> uniform_descriptors, image_descriptors;
            // @TODO: temporary
    };
};  // namespace mag
