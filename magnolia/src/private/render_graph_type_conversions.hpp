#pragma once

#include <vulkan/vulkan_handles.hpp>

namespace mag
{
    enum class AttachmentState;
    vk::AttachmentLoadOp mag_attachment_state_to_vk(const AttachmentState state);
};  // namespace mag
