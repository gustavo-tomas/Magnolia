#pragma once

#include <vector>

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
            vk::RenderPass render_pass;
            vk::Framebuffer frame_buffer;
            vk::PipelineBindPoint pipeline_bind_point;
            vk::Rect2D render_area = {};
            std::vector<vk::ClearValue> clear_values;
    };

    class RenderPass
    {
        public:
            virtual ~RenderPass() = default;

            virtual void initialize(const uvec2& /* size */){};
            virtual void shutdown(){};

            virtual void before_pass(CommandBuffer& /* command_buffer */){};
            virtual void render(CommandBuffer& /* command_buffer */){};
            virtual void after_pass(CommandBuffer& /* command_buffer */){};

            virtual Pass& get_pass() = 0;
    };

    class StandardRenderPass : public RenderPass
    {
        public:
            virtual void initialize(const uvec2& size) override;
            virtual void shutdown() override;

            virtual void before_pass(CommandBuffer& command_buffer) override;
            virtual void render(CommandBuffer& command_buffer) override;
            virtual void after_pass(CommandBuffer& command_buffer) override;

            virtual Pass& get_pass() override { return pass; };
            const Image& get_draw_image() const { return draw_image; };
            f32 get_render_scale() const { return render_scale; };
            uvec3 get_draw_size() const { return draw_size; };
            void set_render_scale(const f32 scale);

            void on_resize(const uvec2& size);

            // @TODO: temporary
            void set_camera(Camera* camera);

        private:
            Pass pass = {};
            Pipeline triangle_pipeline;
            Shader triangle_vs, triangle_fs;
            Image draw_image, depth_image;
            uvec3 draw_size;
            f32 render_scale = 1.0;
            vk::Framebuffer frame_buffer;

            // @TODO: temporary
            struct CameraData
            {
                    mat4 view;        // 64 bytes (16 x 4)
                    mat4 projection;  // 64 bytes (16 x 4)
            };

            Mesh triangle;
            Buffer camera_buffer;
            vk::DescriptorSet descriptor_set;
            vk::DescriptorSetLayout set_layout;
            Camera* camera;
            // @TODO: temporary
    };
};  // namespace mag
