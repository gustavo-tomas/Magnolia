#pragma once

#include <functional>

#include "core/assert.hpp"
#include "core/types.hpp"
#include "math/types.hpp"
#include "private/vulkan_fwd.hpp"

namespace mag
{
// Graphics macros
#define VK_CHECK(result) ASSERT(result == vk::Result::eSuccess, "Vk check failed")
#define VK_CAST(vk_result) static_cast<vk::Result>(vk_result) /* i.e. VK_SUCCESS -> vk::Result::eSuccess */

    class Window;
    class CommandBuffer;
    class DescriptorAllocator;
    class DescriptorLayoutCache;
    class FrameProvider;
    class RendererImage;

    struct Frame;
    struct ProfileResult;

    struct ContextCreateOptions
    {
            Window& window;
            str application_name = "Magnolia";
            str engine_name = "Magnolia";
    };

    enum class ImageFormat
    {
        Srgb,
        Float
    };

    class Context
    {
        public:
            Context(const ContextCreateOptions& options);
            ~Context();

            void recreate_swapchain(const math::uvec2& size);
            void recreate_swapchain(const math::uvec2& size, const vk::PresentModeKHR present_mode);
            b8 begin_frame();
            b8 end_frame(const RendererImage& image, const vk::Extent3D& extent);
            void submit_commands_immediate(std::function<void(CommandBuffer& cmd)>&& function);

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
            const vk::Fence& get_upload_fence() const;
            const std::vector<vk::Image>& get_swapchain_images() const;
            const std::vector<vk::ImageView>& get_swapchain_image_views() const;
            const VmaAllocator& get_allocator() const;
            const ProfileResult& get_timestamp() const;
            const u32& get_curr_frame_number() const;
            const vk::PhysicalDeviceDescriptorBufferPropertiesEXT& get_descriptor_buffer_properties() const;

            vk::Format get_supported_format(const std::vector<vk::Format>& candidates, const vk::ImageTiling tiling,
                                            const vk::FormatFeatureFlags features) const;
            vk::Format get_supported_color_format(const ImageFormat desired_format) const;
            vk::Format get_supported_depth_format() const;
            vk::SampleCountFlags get_available_msaa_samples() const;

            Frame& get_curr_frame();
            DescriptorLayoutCache& get_descriptor_layout_cache();
            DescriptorAllocator& get_descriptor_allocator();

            u32 get_queue_family_index() const;
            u32 get_swapchain_image_index() const;
            u32 get_frame_count() const;

        private:
            struct IMPL;
            unique<IMPL> impl;
    };

    Context& get_context();
};  // namespace mag
