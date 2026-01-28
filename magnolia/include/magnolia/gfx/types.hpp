#pragma once

#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"

namespace mag
{
    namespace gfx
    {
        // Keep it simple
        using ShaderHandle = u32;
        using VertexBufferHandle = u32;
        using IndexBufferHandle = u32;
        using TextureHandle = u32;

        struct GfxOptions
        {
                math::uvec2 resolution = math::uvec2(1280, 720);
        };

        enum class Result : u8
        {
            Success,
            ErrorOutOfDate,
            SubOptimal
        };

        enum class PresentMode : u8
        {
            Immediate,
            Mailbox,
            Fifo
        };

        enum class QueueType : u8
        {
            Present,
            Graphics,
            Compute,
            Transfer
        };

        enum class Format : u8
        {
            Undefined,
            R8_UNORM,
            R8G8B8A8_UNORM,
            B8G8R8A8_UNORM,
            R8G8B8A8_SRGB,
            B8G8R8A8_SRGB,
            R16G16B16A16_SFLOAT,
            R32_UINT,
            R32G32_SFLOAT,
            R32G32B32_SFLOAT,
            R32G32B32A32_SFLOAT,
            D32_SFLOAT,
            D24_UNORM_S8_UINT
        };

        enum class TextureType : u8
        {
            Texture1D,
            Texture2D,
            Texture3D
        };

        enum class TextureViewType : u8
        {
            Texture1D,
            Texture2D,
            Texture3D,
            TextureCube,
            Texture1DArray,
            Texture2DArray,
            TextureCubeArray
        };

        enum class TextureUsage : u8
        {
            TransferSrc = 1 << 0,
            TransferDst = 1 << 1,
            Sampled = 1 << 2,
            Storage = 1 << 3,
            ColorAttachment = 1 << 4,
            DepthStencilAttachment = 1 << 5
        };

        enum class TextureLayout : u8
        {
            Undefined,
            ColorAttachment,
            Present,
            TransferSrc,
            TransferDst,
            ShaderReadOnly
        };

        enum class TextureAspect : u8
        {
            None = 0,
            Color = 1 << 0,
            Depth = 1 << 1,
            Stencil = 1 << 2
        };

        enum class Filter : u8
        {
            Nearest,
            Linear
        };

        enum class SamplerMipMapMode : u8
        {
            Nearest,
            Linear
        };

        enum class SamplerAddressMode : u8
        {
            Repeat,
            MirroredRepeat,
            ClampToEdge,
            ClampToBorder,
            MirrorClampToEdge
        };

        enum class SampleCount : u8
        {
            e1 = 1 << 0,
            e2 = 1 << 1,
            e4 = 1 << 2,
            e8 = 1 << 3,
            e16 = 1 << 4
        };

        enum class AccessMask : u8
        {
            None = 0,
            ColorAttachmentWrite = 1 << 0,
            TransferRead = 1 << 1,
            TransferWrite = 1 << 2,
            MemoryRead = 1 << 3,
            MemoryWrite = 1 << 4,
            ShaderRead = 1 << 5
        };

        enum class PipelineStage : u8
        {
            TopOfPipe = 1 << 0,
            ColorAttachmentOutput = 1 << 1,
            BottomOfPipe = 1 << 2,
            Transfer = 1 << 3,
            FragmentShader = 1 << 4,
            AllCommands = 1 << 5
        };

        enum class PrimitiveTopology : u8
        {
            TriangleList,
            TriangleStrip,
            LineList
        };

        enum class CommandBufferLevel : u8
        {
            Primary,
            Secondary
        };

        enum class RenderingAttachmentType : u8
        {
            Color,
            Depth
        };

        enum class ShaderStage : u8
        {
            Vertex = 1 << 0,
            Fragment = 1 << 1
        };

        enum class BufferUsage : u8
        {
            Vertex = 1 << 0,
            Index = 1 << 1,
            Uniform = 1 << 2,
            Storage = 1 << 3,
            TransferSrc = 1 << 4,
            TransferDst = 1 << 5
        };

        enum class MemoryUsage : u8
        {
            Auto,
            PreferHost,
            PreferDevice
        };

        enum class DescriptorType : u8
        {
            Uniform,
            Storage,
            CombinedImageSampler
        };

        enum class VertexInputRate : u8
        {
            Vertex,
            Instance
        };

        enum class BlendOp : u8
        {
            Add
        };

        enum class BlendFactor : u8
        {
            One,
            SrcAlpha,
            OneMinusSrcAlpha
        };
    };  // namespace gfx
};  // namespace mag

ENABLE_BITMASK_OPERATORS(mag::gfx::BufferUsage);
ENABLE_BITMASK_OPERATORS(mag::gfx::TextureUsage);
ENABLE_BITMASK_OPERATORS(mag::gfx::TextureAspect);
ENABLE_BITMASK_OPERATORS(mag::gfx::AccessMask);
ENABLE_BITMASK_OPERATORS(mag::gfx::PipelineStage);
ENABLE_BITMASK_OPERATORS(mag::gfx::ShaderStage);
