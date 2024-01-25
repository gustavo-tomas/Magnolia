#pragma once

#include "core/types.hpp"

namespace mag
{
    class Shader
    {
        public:
            void initialize(const str& file, const vk::ShaderStageFlagBits stage);
            void shutdown();

            const str& get_file() const { return file; };
            const vk::ShaderModule& get_handle() const { return module; };
            vk::ShaderStageFlagBits get_stage() const { return stage; };

        private:
            vk::ShaderModule module = {};
            vk::ShaderStageFlagBits stage = {};
            str file = {};
    };
};  // namespace mag
