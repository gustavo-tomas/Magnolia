#pragma once

#include <vulkan/vulkan.hpp>

#include "core/assert.hpp"
#include "core/types.hpp"
#include "core/window.hpp"
#include "renderer/command.hpp"
#include "renderer/descriptors.hpp"
#include "renderer/frame.hpp"
#include "tools/profiler.hpp"
#include "vk_mem_alloc.h"

// Graphics macros
#define VK_CHECK(result) ASSERT(result == vk::Result::eSuccess, "Vk check failed")
#define VK_CAST(vk_result) static_cast<vk::Result>(vk_result) /* i.e. VK_SUCCESS -> vk::Result::eSuccess */

namespace mag
{
    struct ContextCreateOptions
    {
            Window& window;
            str application_name = "Magnolia";
            str engine_name = "Magnolia";
    };

    class Context
    {
        public:
            Context(const ContextCreateOptions& options);
            ~Context();

            void recreate_swapchain(const uvec2& size);
            void recreate_swapchain(const uvec2& size, const vk::PresentModeKHR present_mode);
            void begin_frame();
            b8 end_frame(const RendererImage& image, const vk::Extent3D& extent);
            void submit_commands_immediate(std::function<void(CommandBuffer cmd)>&& function);

            void begin_timestamp();
            void end_timestamp();
            void calculate_timestamp();

            const vk::Instance& get_instance() const;
            const vk::Device& get_device() const;
            const vk::PhysicalDevice& get_physical_device() const;
            const vk::Queue& get_graphics_queue() const;
            const vk::SurfaceKHR& get_surface() const;
            const vk::SurfaceFormatKHR& get_surface_format() const;
            const vk::Extent2D& get_surface_extent() const;
            const vk::SwapchainKHR& get_swapchain() const;
            const vk::Format& get_swapchain_image_format() const;
            const vk::CommandPool& get_command_pool() const;
            const vk::Fence& get_upload_fence() const;
            const std::vector<vk::Image>& get_swapchain_images() const;
            const std::vector<vk::ImageView>& get_swapchain_image_views() const;
            const VmaAllocator& get_allocator() const;
            const ProfileResult& get_timestamp() const;
            const u32& get_curr_frame_number() const;
            const vk::PhysicalDeviceDescriptorBufferPropertiesEXT& get_descriptor_buffer_properties() const;

            vk::Format get_supported_format(const std::vector<vk::Format>& candidates, const vk::ImageTiling tiling,
                                            const vk::FormatFeatureFlags features) const;
            vk::Format get_supported_depth_format() const;

            Frame& get_curr_frame();
            DescriptorLayoutCache& get_descriptor_layout_cache();
            DescriptorAllocator& get_descriptor_allocator();

            vk::SampleCountFlagBits get_msaa_samples() const;
            u32 get_queue_family_index() const;
            u32 get_swapchain_image_index() const;
            u32 get_frame_count() const;

        private:
            vk::Instance instance;
            vk::SurfaceKHR surface;
            vk::SurfaceFormatKHR surface_format;
            vk::Extent2D surface_extent;
            vk::PhysicalDevice physical_device;
            vk::PhysicalDeviceDescriptorBufferPropertiesEXT descriptor_buffer_properties;
            vk::PhysicalDeviceProperties2 physical_device_properties;
            vk::Device device;
            vk::PresentModeKHR surface_present_mode;
            vk::SwapchainKHR swapchain;
            vk::Queue graphics_queue;
            vk::CommandPool command_pool, immediate_command_pool;
            vk::Fence upload_fence;
            vk::DebugUtilsMessengerEXT debug_utils_messenger;
            vk::QueryPool query_pool;
            vk::SampleCountFlagBits msaa_samples;

            std::vector<vk::Image> swapchain_images;
            std::vector<vk::ImageView> swapchain_image_views;

            u32 api_version = {};
            u32 queue_family_index = {};
            u32 present_image_count = {};
            u32 frame_count = {};
            const u32 query_count = 2;
            b8 is_query_supported = {};
            f32 timestamp_period = {};
            ProfileResult timestamp = {};

            FrameProvider frame_provider;
            VmaAllocator allocator = {};
            unique<DescriptorLayoutCache> descriptor_layout_cache;
            unique<DescriptorAllocator> descriptor_allocator;

            CommandBuffer submit_command_buffer;
    };

    Context& get_context();
};  // namespace mag
