#pragma once

#include <vulkan/vulkan.hpp>

#include "core/types.hpp"

#define VK_CHECK(result) ASSERT(result == vk::Result::eSuccess, "Vk check failed")

namespace mag
{
    struct ContextCreateOptions
    {
            str application_name = "Magnolia";
            str engine_name = "Magnolia";

            std::vector<const char*> extensions;
            std::vector<const char*> layers;
    };

    class Context
    {
        public:
            void create_instance(const ContextCreateOptions& options);
            void destroy();

            const vk::Instance& get_instance() const { return this->instance; };

        private:
            vk::Instance instance;
            u32 api_version = {};
    };
};  // namespace mag
