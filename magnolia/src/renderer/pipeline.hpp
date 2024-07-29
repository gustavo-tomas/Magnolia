#pragma once

#include <vulkan/vulkan.hpp>

#include "nlohmann/json.hpp"
#include "renderer/shader.hpp"

namespace mag
{
    using namespace mag::math;
    using json = nlohmann::ordered_json;

    class Pipeline
    {
        public:
            Pipeline(const Shader& shader, const vk::PipelineRenderingCreateInfo pipeline_rendering_create_info,
                     const json pipeline_data);

            ~Pipeline();

            void bind();

            const vk::PipelineLayout& get_layout() const { return pipeline_layout; };

        private:
            vk::Pipeline pipeline;
            vk::PipelineLayout pipeline_layout;
    };
};  // namespace mag
