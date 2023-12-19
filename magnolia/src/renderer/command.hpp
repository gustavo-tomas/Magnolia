#pragma once

#include <vulkan/vulkan.hpp>

namespace mag
{
    class CommandBuffer
    {
        public:
            void initialize(const vk::CommandPool& pool, const vk::CommandBufferLevel level);

            void begin();
            void end();

            const vk::CommandBuffer& get_handle() const { return this->command_buffer; }

        private:
            vk::CommandBuffer command_buffer;
    };
};  // namespace mag
