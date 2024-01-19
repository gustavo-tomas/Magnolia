#pragma once

#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "core/window.hpp"
#include "renderer/command.hpp"
#include "renderer/frame.hpp"
#include "vk_mem_alloc.h"

#define VK_CHECK(result) ASSERT(result == vk::Result::eSuccess, "Vk check failed")
#define VK_CAST(vk_result) static_cast<vk::Result>(vk_result) /* i.e. VK_SUCCESS -> vk::Result::eSuccess */
#define MAG_TIMEOUT 1'000'000'000                             /* 1 second in nanoseconds */

namespace mag
{
    struct ContextCreateOptions
    {
            Window& window;
            str application_name = "Magnolia";
            str engine_name = "Magnolia";

            std::vector<const char*> instance_extensions;
            std::vector<const char*> device_extensions;
            std::vector<const char*> validation_layers;

            vk::PhysicalDeviceType preferred_device_type = vk::PhysicalDeviceType::eDiscreteGpu;
            u32 frame_count = 3;  // 3 for triple buffering
    };

    class Context
    {
        public:
            void initialize(const ContextCreateOptions& options);
            void shutdown();

            void recreate_swapchain(const glm::uvec2& size, const vk::PresentModeKHR present_mode);
            b8 begin_frame();
            b8 end_frame();

            const vk::Instance& get_instance() const { return this->instance; };
            const vk::Device& get_device() const { return this->device; };
            const vk::PhysicalDevice& get_physical_device() const { return this->physical_device; };
            const vk::Queue& get_graphics_queue() const { return this->graphics_queue; };
            const vk::SurfaceKHR& get_surface() const { return this->surface; };
            const vk::SurfaceFormatKHR& get_surface_format() const { return this->surface_format; };
            const vk::Extent2D& get_surface_extent() const { return this->surface_extent; };
            const vk::SwapchainKHR& get_swapchain() const { return this->swapchain; };
            const vk::Format& get_swapchain_image_format() const { return this->surface_format.format; };
            const vk::CommandPool& get_command_pool() const { return this->command_pool; };
            const vk::Semaphore& get_present_semaphore() const { return this->present_semaphore; };
            const vk::Semaphore& get_render_semaphore() const { return this->render_semaphore; };
            const std::vector<vk::Image>& get_swapchain_images() const { return this->swapchain_images; };
            const std::vector<vk::ImageView>& get_swapchain_image_views() const { return this->swapchain_image_views; };
            const CommandBuffer& get_command_buffer() const { return this->command_buffer; };
            const VmaAllocator& get_allocator() const { return this->allocator; };
            Frame& get_curr_frame() { return this->frame_provider.get_current_frame(); };

            vk::SampleCountFlagBits get_msaa_samples() const { return this->msaa_samples; };
            u32 get_swapchain_image_index() const { return frame_provider.get_swapchain_image_index(); };

        private:
            vk::Instance instance;
            vk::SurfaceKHR surface;
            vk::SurfaceFormatKHR surface_format;
            vk::Extent2D surface_extent;
            vk::PhysicalDevice physical_device;
            vk::Device device;
            vk::PresentModeKHR surface_present_mode;
            vk::SwapchainKHR swapchain;
            vk::Queue graphics_queue;
            vk::Fence upload_fence;
            vk::Semaphore present_semaphore, render_semaphore;
            vk::CommandPool command_pool;
            vk::DispatchLoaderDynamic dynamic_loader;
            vk::DebugUtilsMessengerEXT debug_utils_messenger;
            vk::SampleCountFlagBits msaa_samples;

            std::vector<vk::Image> swapchain_images;
            std::vector<vk::ImageView> swapchain_image_views;

            u32 api_version = {};
            u32 queue_family_index = {};
            u32 present_image_count = {};

            CommandBuffer command_buffer;
            FrameProvider frame_provider;
            VmaAllocator allocator = {};
    };

    Context& get_context();
};  // namespace mag
