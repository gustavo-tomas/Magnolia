#pragma once

// Forward declarations for vulkan types (because khronos can't do them properly)

// VMA
typedef struct VmaAllocator_T* VmaAllocator;

// SPIRV
struct SpvReflectBlockVariable;
struct SpvReflectDescriptorBinding;
struct SpvReflectShaderModule;

namespace vk
{
    union ClearValue;

    enum class AttachmentLoadOp;
    enum class BlendFactor;
    enum class BlendOp;
    enum class CommandBufferLevel;
    enum class DescriptorType;
    enum class Filter;
    enum class Format;
    enum class ImageLayout;
    enum class ImageTiling;
    enum class PipelineBindPoint;
    enum class PolygonMode;
    enum class PresentModeKHR;
    enum class PrimitiveTopology;
    enum class SamplerAddressMode;
    enum class SamplerMipmapMode;

    enum class BufferUsageFlagBits : unsigned int;
    enum class CullModeFlagBits : unsigned int;
    enum class FormatFeatureFlagBits : unsigned int;
    enum class ImageAspectFlagBits : unsigned int;
    enum class ImageUsageFlagBits : unsigned int;
    enum class SampleCountFlagBits : unsigned int;
    enum class ShaderStageFlagBits : unsigned int;

    struct Extent2D;
    struct Extent3D;
    struct DescriptorBufferInfo;
    struct DescriptorImageInfo;
    struct DescriptorSetLayoutBinding;
    struct DescriptorSetLayoutCreateInfo;
    struct PhysicalDeviceDescriptorBufferPropertiesEXT;
    struct RenderingInfo;
    struct SurfaceFormatKHR;
    struct VertexInputAttributeDescription;
    struct VertexInputBindingDescription;
    struct WriteDescriptorSet;

    class Buffer;
    class CommandBuffer;
    class CommandPool;
    class DescriptorPool;
    class DescriptorSet;
    class DescriptorSetLayout;
    class Device;
    class Fence;
    class Image;
    class ImageView;
    class Instance;
    class PhysicalDevice;
    class PipelineLayout;
    class Queue;
    class Semaphore;
    class ShaderModule;
    class SurfaceKHR;
    class SwapchainKHR;

    template <typename BitType>
    class Flags;

    using BufferUsageFlags = Flags<BufferUsageFlagBits>;
    using CullModeFlags = Flags<CullModeFlagBits>;
    using FormatFeatureFlags = Flags<FormatFeatureFlagBits>;
    using ImageAspectFlags = Flags<ImageAspectFlagBits>;
    using ImageUsageFlags = Flags<ImageUsageFlagBits>;
    using ShaderStageFlags = Flags<ShaderStageFlagBits>;
};  // namespace vk
