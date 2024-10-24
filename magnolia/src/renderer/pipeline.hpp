#pragma once

#include <vulkan/vulkan_handles.hpp>

#include "renderer/shader.hpp"

namespace mag
{
    using namespace mag::math;

    class Pipeline
    {
        public:
            Pipeline(const Shader& shader);
            ~Pipeline();

            void bind();

            const vk::PipelineLayout& get_layout() const;

        private:
            vk::Pipeline pipeline;
            vk::PipelineLayout pipeline_layout;
    };
};  // namespace mag
