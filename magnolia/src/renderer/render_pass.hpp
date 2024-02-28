#pragma once

#include "camera/camera.hpp"
#include "renderer/image.hpp"
#include "renderer/model.hpp"
#include "renderer/pipeline.hpp"

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

    class RenderPass
    {
        public:
            virtual ~RenderPass() = default;

            virtual void initialize(const uvec2& /* size */){};
            virtual void shutdown(){};

            virtual void before_render(CommandBuffer& /* command_buffer */){};
            virtual void render(CommandBuffer& /* command_buffer */, const Mesh& /* mesh */){};
            virtual void after_render(CommandBuffer& /* command_buffer */){};

            virtual Pass& get_pass() = 0;
    };

    class StandardRenderPass : public RenderPass
    {
        public:
            virtual void initialize(const uvec2& size) override;
            virtual void shutdown() override;

            virtual void before_render(CommandBuffer& command_buffer) override;
            virtual void render(CommandBuffer& command_buffer, const Mesh& mesh) override;
            virtual void after_render(CommandBuffer& command_buffer) override;

            virtual Pass& get_pass() override { return pass; };
            const Image& get_target_image() const { return resolve_image; };
            f32 get_render_scale() const { return render_scale; };
            uvec3 get_draw_size() const { return draw_size; };
            void set_render_scale(const f32 scale);

            void on_resize(const uvec2& size);

            // @TODO: temporary
            void set_camera(Camera* camera);

        private:
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

            Buffer camera_buffer;
            vk::DescriptorSet descriptor_set;
            vk::DescriptorSetLayout set_layout;
            Camera* camera;
            // @TODO: temporary
    };
};  // namespace mag
