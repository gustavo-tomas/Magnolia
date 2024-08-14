#pragma once

#include "core/logger.hpp"
#include "core/types.hpp"
#include "renderer/render_graph.hpp"

namespace mag
{
    inline vk::AttachmentLoadOp mag_attachment_state_to_vk(const AttachmentState state)
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
