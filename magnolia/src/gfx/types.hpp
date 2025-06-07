#pragma once

#include "core/types.hpp"
#include "math/types.hpp"

namespace mag
{
    namespace gfx
    {
        // Keep it simple
        typedef u32 ShaderHandle;
        typedef u32 VertexBufferHandle;
        typedef u32 IndexBufferHandle;
        typedef u32 TextureHandle;

        struct GfxOptions
        {
                math::uvec2 resolution = math::uvec2(1280, 720);
        };

        enum class Result
        {
            Success,
            ErrorOutOfDate,
            SubOptimal
        };

        enum class PresentMode
        {
            Immediate,
            Mailbox,
            Fifo
        };

        enum class QueueType
        {
            Present,
            Graphics,
            Compute,
            Transfer
        };

        enum class Format
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

        enum class TextureType
        {
            Texture1D,
            Texture2D,
            Texture3D
        };

        enum class TextureViewType
        {
            Texture1D,
            Texture2D,
            Texture3D,
            TextureCube,
            Texture1DArray,
            Texture2DArray,
            TextureCubeArray
        };

        enum class TextureUsage : u32
        {
            TransferSrc = 1 << 0,
            TransferDst = 1 << 1,
            Sampled = 1 << 2,
            Storage = 1 << 3,
            ColorAttachment = 1 << 4,
            DepthStencilAttachment = 1 << 5
        };

        enum class TextureLayout
        {
            Undefined,
            ColorAttachment,
            Present,
            TransferSrc,
            TransferDst,
            ShaderReadOnly
        };

        enum class TextureAspect
        {
            None = 0,
            Color = 1 << 0,
            Depth = 1 << 1,
            Stencil = 1 << 2
        };

        enum class Filter
        {
            Nearest,
            Linear
        };

        enum class SamplerMipMapMode
        {
            Nearest,
            Linear
        };

        enum class SamplerAddressMode
        {
            Repeat,
            MirroredRepeat,
            ClampToEdge,
            ClampToBorder,
            MirrorClampToEdge
        };

        enum class SampleCount
        {
            e1 = 1 << 0,
            e2 = 1 << 1,
            e4 = 1 << 2,
            e8 = 1 << 3,
            e16 = 1 << 4
        };

        enum class AccessMask
        {
            None = 0,
            ColorAttachmentWrite = 1 << 0,
            TransferRead = 1 << 1,
            TransferWrite = 1 << 2,
            MemoryRead = 1 << 3,
            MemoryWrite = 1 << 4,
            ShaderRead = 1 << 5
        };

        enum class PipelineStage
        {
            TopOfPipe = 1 << 0,
            ColorAttachmentOutput = 1 << 1,
            BottomOfPipe = 1 << 2,
            Transfer = 1 << 3,
            FragmentShader = 1 << 4,
            AllCommands = 1 << 5
        };

        enum class PrimitiveTopology
        {
            TriangleList,
            TriangleStrip,
            LineList
        };

        enum class CommandBufferLevel
        {
            Primary,
            Secondary
        };

        enum class RenderingAttachmentType
        {
            Color,
            Depth
        };

        enum class ShaderStage
        {
            Vertex = 1 << 0,
            Fragment = 1 << 1
        };

        enum class BufferUsage
        {
            Vertex = 1 << 0,
            Index = 1 << 1,
            Uniform = 1 << 2,
            Storage = 1 << 3,
            TransferSrc = 1 << 4,
            TransferDst = 1 << 5
        };

        enum class MemoryUsage
        {
            Auto,
            PreferHost,
            PreferDevice
        };

        enum class DescriptorType
        {
            Uniform,
            Storage,
            CombinedImageSampler
        };

        enum class VertexInputRate
        {
            Vertex,
            Instance
        };

        enum class BlendOp
        {
            Add
        };

        enum class BlendFactor
        {
            One,
            SrcAlpha,
            OneMinusSrcAlpha
        };
    };  // namespace gfx
};      // namespace mag

ENABLE_BITMASK_OPERATORS(mag::gfx::BufferUsage);
ENABLE_BITMASK_OPERATORS(mag::gfx::TextureUsage);
ENABLE_BITMASK_OPERATORS(mag::gfx::TextureAspect);
ENABLE_BITMASK_OPERATORS(mag::gfx::AccessMask);
ENABLE_BITMASK_OPERATORS(mag::gfx::PipelineStage);
ENABLE_BITMASK_OPERATORS(mag::gfx::ShaderStage);
