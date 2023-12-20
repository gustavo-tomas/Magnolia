#pragma once

#include <vulkan/vulkan.hpp>

namespace mag
{
    struct Pass;

    class CommandBuffer
    {
        public:
            void initialize(const vk::CommandPool& pool, const vk::CommandBufferLevel level);

            void begin();
            void end();
            void begin_pass(const Pass& pass);
            void end_pass(const Pass& pass);

            const vk::CommandBuffer& get_handle() const { return this->command_buffer; }

        private:
            vk::CommandBuffer command_buffer;
    };
};  // namespace mag
