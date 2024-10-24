#pragma once

#include <map>

#include "renderer/renderer_image.hpp"

namespace mag
{
    using namespace mag::math;

    enum class AttachmentType
    {
        Color,
        Depth
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
            vk::RenderingInfo rendering_info;
            vk::RenderingAttachmentInfo color_attachment;
            vk::RenderingAttachmentInfo depth_attachment;

            vk::ClearValue color_clear_value = vk::ClearColorValue(0.0f, 1.0f, 1.0f, 1.0f);
            vk::ClearValue depth_clear_value = vk::ClearDepthStencilValue(1.0f);
            uvec2 size = {};
    };

    class RenderGraph;
    class RenderGraphPass
    {
        public:
            RenderGraphPass(const str& name);
            virtual ~RenderGraphPass() = default;

            virtual void on_render(RenderGraph& render_graph) { (void)render_graph; };

            const PerformanceResults& get_performance_results() const { return performance_results; };
            const str& get_name() const { return name; };

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
            RenderGraph() = default;
            ~RenderGraph();

            void add_pass(RenderGraphPass* pass);
            void set_output_attachment(const str& attachment_name);

            void build();
            void execute();

            RendererImage& get_attachment(const str& attachment_name)
            {
                const u32 curr_frame = get_context().get_curr_frame_number();
                return *attachments[attachment_name][curr_frame].texture;
            };
            RendererImage& get_output_attachment() { return get_attachment(output_attachment_name); };
            const std::vector<RenderGraphPass*>& get_passes() const { return passes; };

        private:
            void execute_render_pass(RenderGraphPass* render_pass);

            std::vector<RenderGraphPass*> passes;
            std::map<str, std::vector<Attachment>> attachments;  // One per frame in flight
            str output_attachment_name;
    };
};  // namespace mag
