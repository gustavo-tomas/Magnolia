#pragma once

#include <vector>

#include "renderer/image.hpp"

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
            Image texture;
            vk::ImageLayout curr_layout;
    };

    struct Pass
    {
            vk::RenderingInfo rendering_info;
            vk::RenderingAttachmentInfo color_attachment;
            vk::RenderingAttachmentInfo depth_attachment;

            vk::ClearValue color_clear_value = vk::ClearColorValue(0.0f, 1.0f, 1.0f, 1.0f);
            vk::ClearValue depth_clear_value = vk::ClearDepthStencilValue(1.0f);
            uvec2 size = {};

            Statistics statistics = {};
    };

    class RenderGraph;
    class RenderGraphPass
    {
        public:
            RenderGraphPass(const str& name, const uvec2& size);
            virtual ~RenderGraphPass() = default;

            virtual void on_render(RenderGraph& render_graph) { (void)render_graph; };

            // @TODO: temp?
            Pass pass;

        protected:
            void add_input_attachment(const str& attachment_name, const AttachmentType attachment_type,
                                      const uvec2& size,
                                      const AttachmentState attachment_state = AttachmentState::Clear);

            void add_output_attachment(const str& attachment_name, const AttachmentType attachment_type,
                                       const uvec2& size,
                                       const AttachmentState attachment_state = AttachmentState::Clear);

        private:
            friend class RenderGraph;

            void add_attachment(const AttachmentDescription& attachment_description);

            const str name;
            const uvec2 size;
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

            Image& get_attachment(const str& attachment_name) { return attachments[attachment_name].texture; };
            Image& get_output_attachment() { return get_attachment(output_attachment_name); };

        private:
            void execute_render_pass(RenderGraphPass* render_pass);

            std::vector<RenderGraphPass*> passes;
            std::map<str, Attachment> attachments;
            str output_attachment_name;
    };
};  // namespace mag
