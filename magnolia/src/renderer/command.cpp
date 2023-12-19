#include "renderer/command.hpp"

#include "renderer/context.hpp"

namespace mag
{
    void CommandBuffer::initialize(const vk::CommandPool& pool, const vk::CommandBufferLevel level)
    {
        vk::CommandBufferAllocateInfo cmd_alloc_info(pool, level, 1);
        this->command_buffer = get_context().get_device().allocateCommandBuffers(cmd_alloc_info).front();
    }

    void CommandBuffer::begin()
    {
        vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        this->command_buffer.begin(begin_info);
    }

    void CommandBuffer::end() { this->command_buffer.end(); }
};  // namespace mag
