#pragma once

#include <memory>
#include <vector>

#include "camera/camera.hpp"
#include "renderer/descriptors.hpp"
#include "renderer/image.hpp"
#include "renderer/model.hpp"
#include "renderer/pipeline.hpp"

// @TODO: review this whole system

namespace mag
{
    using namespace mag::math;

    class CommandBuffer;

    struct Pass
    {
            ~Pass()
            {
                delete rendering_info;
                delete color_attachment;
                delete depth_attachment;
            }

            vk::RenderingInfo* rendering_info;
            vk::RenderingAttachmentInfo* color_attachment;
            vk::RenderingAttachmentInfo* depth_attachment;
    };

    class StandardRenderPass
    {
        public:
            void initialize(const uvec2& size);
            void shutdown();

            void before_render(CommandBuffer& command_buffer);
            void render(CommandBuffer& command_buffer, const Camera& camera, const std::vector<Model>& models);
            void after_render(CommandBuffer& command_buffer);

            // @TODO: temp
            void add_model(const Model& model);
            void set_camera();

            Pass& get_pass() { return pass; };
            const Image& get_target_image() const { return resolve_image; };
            f32 get_render_scale() const { return render_scale; };
            uvec3 get_draw_size() const { return draw_size; };
            void set_render_scale(const f32 scale);

            void on_resize(const uvec2& size);

        private:
            void add_uniform(const u64 buffer_size);

            Pass pass = {};
            Pipeline triangle_pipeline, grid_pipeline;
            Shader triangle_vs, triangle_fs, grid_vs, grid_fs;
            Image draw_image, depth_image, resolve_image;
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

            std::vector<Buffer> data_buffers;
            std::vector<Image> textures;
            Descriptor uniform_descriptor, image_descriptor;
            // @TODO: temporary
    };
};  // namespace mag
