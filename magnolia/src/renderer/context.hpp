#pragma once

#include <memory>
#include <vulkan/vulkan.hpp>

#include "core/types.hpp"
#include "core/window.hpp"
#include "renderer/command.hpp"
#include "renderer/descriptors.hpp"
#include "renderer/frame.hpp"
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

    struct Timestamp
    {
            f64 begin = 0;
            f64 end = 0;
    };

    class Context
    {
        public:
            Context(const ContextCreateOptions& options);
            ~Context();

            void recreate_swapchain(const uvec2& size);
            void recreate_swapchain(const uvec2& size, const vk::PresentModeKHR present_mode);
            void begin_frame();
            b8 end_frame(const Image& image, const vk::Extent3D& extent);
            void submit_commands_immediate(std::function<void(CommandBuffer cmd)>&& function);

            void begin_timestamp();
            void end_timestamp();
            void calculate_timestamp();

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
            const vk::Fence& get_upload_fence() const { return this->upload_fence; };
            const std::vector<vk::Image>& get_swapchain_images() const { return this->swapchain_images; };
            const std::vector<vk::ImageView>& get_swapchain_image_views() const { return this->swapchain_image_views; };
            const VmaAllocator& get_allocator() const { return this->allocator; };
            const Timestamp& get_timestamp() const { return this->timestamp; };
            const u32& get_curr_frame_number() const { return this->frame_provider.get_current_frame_number(); };

            const vk::PhysicalDeviceDescriptorBufferPropertiesEXT& get_descriptor_buffer_properties() const
            {
                return this->descriptor_buffer_properties;
            };

            vk::Format get_supported_format(const std::vector<vk::Format>& candidates, const vk::ImageTiling tiling,
                                            const vk::FormatFeatureFlags features) const;
            vk::Format get_supported_depth_format() const;

            Frame& get_curr_frame() { return this->frame_provider.get_current_frame(); };
            DescriptorLayoutCache& get_descriptor_layout_cache() { return this->descriptor_layout_cache; };
            DescriptorCache& get_descriptor_cache() { return *this->descriptor_cache; };

            vk::SampleCountFlagBits get_msaa_samples() const { return this->msaa_samples; };
            u32 get_queue_family_index() const { return this->queue_family_index; };
            u32 get_swapchain_image_index() const { return frame_provider.get_swapchain_image_index(); };
            u32 get_frame_count() const { return frame_count; };

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
            vk::CommandPool command_pool;
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
            Timestamp timestamp = {};

            FrameProvider frame_provider;
            VmaAllocator allocator = {};
            DescriptorLayoutCache descriptor_layout_cache;
            std::unique_ptr<DescriptorCache> descriptor_cache;
            CommandBuffer submit_command_buffer;
    };

    Context& get_context();
};  // namespace mag
