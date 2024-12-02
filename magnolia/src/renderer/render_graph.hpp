#pragma once

#include <map>

#include "math/vec.hpp"
#include "private/vulkan_fwd.hpp"
#include "renderer/renderer_image.hpp"

namespace mag
{
    using namespace mag::math;

    enum class AttachmentType
    {
        Color,
        DepthStencil
    };

    enum class AttachmentStage
    {
        Input,
        Output
    };

    enum class AttachmentState
    {
        Load,
        Clear
    };

    struct AttachmentDescription
    {
            str name;
            AttachmentType type;
            AttachmentStage stage;
            AttachmentState state;
            uvec2 size;
    };

    struct Attachment
    {
            AttachmentDescription description;
            ref<RendererImage> texture;
            vk::ImageLayout curr_layout;
    };

    struct PerformanceResults
    {
            u32 rendered_triangles = 0;
            u32 draw_calls = 0;
    };

    struct Pass
    {
            void* rendering_info = nullptr;
            void* color_attachment = nullptr;
            void* depth_attachment = nullptr;

            vec4 color_clear_value = vec4(0.0f, 1.0f, 1.0f, 1.0f);
            vec2 depth_stencil_clear_value = vec2(1.0f, 0.0f);
            uvec2 size = {};
    };

    class RenderGraph;
    class RenderGraphPass
    {
        public:
            RenderGraphPass(const str& name);
            virtual ~RenderGraphPass();

            virtual void on_render(RenderGraph& render_graph);

            const PerformanceResults& get_performance_results() const;
            const str& get_name() const;

            // @TODO: temp?
            Pass pass;

        protected:
            void add_input_attachment(const str& attachment_name, const AttachmentType attachment_type,
                                      const uvec2& size,
                                      const AttachmentState attachment_state = AttachmentState::Clear);

            void add_output_attachment(const str& attachment_name, const AttachmentType attachment_type,
                                       const uvec2& size,
                                       const AttachmentState attachment_state = AttachmentState::Clear);

            PerformanceResults performance_results;

        private:
            friend class RenderGraph;

            void add_attachment(const AttachmentDescription& attachment_description);

            const str name;
            std::vector<AttachmentDescription> attachment_descriptions;
    };

    class RenderGraph
    {
        public:
            RenderGraph();
            ~RenderGraph();

            void add_pass(RenderGraphPass* pass);
            void set_output_attachment(const str& attachment_name);

            void build();
            void execute();

            RendererImage& get_attachment(const str& attachment_name);
            RendererImage& get_output_attachment();
            const std::vector<RenderGraphPass*>& get_passes() const;

        private:
            void execute_render_pass(RenderGraphPass* render_pass);

            std::vector<RenderGraphPass*> passes;
            std::map<str, std::vector<Attachment>> attachments;  // One per frame in flight
            str output_attachment_name;
    };
};  // namespace mag
