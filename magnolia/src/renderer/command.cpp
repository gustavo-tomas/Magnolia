#include "renderer/command.hpp"

#include "renderer/context.hpp"
#include "renderer/render_pass.hpp"

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

    void CommandBuffer::begin_pass(const Pass& pass)
    {
        if (!pass.render_pass) return;

        vk::RenderPassBeginInfo begin_info(pass.render_pass, pass.frame_buffer, pass.render_area, pass.clear_values);
        this->command_buffer.beginRenderPass(begin_info, vk::SubpassContents::eInline);
    }

    void CommandBuffer::end_pass(const Pass& pass)
    {
        if (pass.render_pass) this->command_buffer.endRenderPass();
    }
};  // namespace mag
