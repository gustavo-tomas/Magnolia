#include "private/render_graph_type_conversions.hpp"

#include "core/assert.hpp"
#include "renderer/render_graph.hpp"

namespace mag
{
    vk::AttachmentLoadOp mag_attachment_state_to_vk(const AttachmentState state)
    {
        switch (state)
        {
            case AttachmentState::Clear:
                return vk::AttachmentLoadOp::eClear;
                break;

            case AttachmentState::Load:
                return vk::AttachmentLoadOp::eLoad;
                break;

            default:
                ASSERT(false, "Invalid AttachmentState");
                return vk::AttachmentLoadOp::eClear;
                break;
        }
    }
};  // namespace mag
