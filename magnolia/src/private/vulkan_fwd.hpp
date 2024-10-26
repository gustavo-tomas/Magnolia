#pragma once

// Forward declarations for vulkan and vma types (because khronos can't do it properly)

typedef struct VmaAllocator_T* VmaAllocator;

namespace vk
{
    union ClearValue;

    struct Extent2D;
    struct Extent3D;

    enum class Filter;
    enum class SamplerAddressMode;
    enum class SamplerMipmapMode;
    enum class AttachmentLoadOp;

    enum class PrimitiveTopology;
    enum class PolygonMode;
    enum class BlendOp;
    enum class BlendFactor;

    enum class Format;
    enum class ImageTiling;
    enum class ImageLayout;
    enum class PresentModeKHR;

    enum class CommandBufferLevel;
    enum class PipelineBindPoint;

    enum class SampleCountFlagBits : unsigned int;
    enum class FormatFeatureFlagBits : unsigned int;
    enum class CullModeFlagBits : unsigned int;
    enum class BufferUsageFlagBits : unsigned int;

    struct Extent2D;
    struct Extent3D;
    struct RenderingInfo;
    struct SurfaceFormatKHR;
    struct PhysicalDeviceDescriptorBufferPropertiesEXT;

    class Buffer;

    class CommandPool;
    class CommandBuffer;
    class PipelineLayout;
    class DescriptorSet;

    class Instance;
    class Device;
    class PhysicalDevice;
    class SwapchainKHR;
    class SurfaceKHR;
    class CommandPool;
    class Fence;
    class Queue;
    class Image;
    class ImageView;

    template <typename BitType>
    class Flags;

    using FormatFeatureFlags = Flags<FormatFeatureFlagBits>;
    using CullModeFlags = Flags<CullModeFlagBits>;
    using BufferUsageFlags = Flags<BufferUsageFlagBits>;
};  // namespace vk
