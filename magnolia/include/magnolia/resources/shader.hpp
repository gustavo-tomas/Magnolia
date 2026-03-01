#pragma once

#include <vector>

#include "magnolia/core/types.hpp"
#include "magnolia/resources/resource.hpp"

namespace mag
{
    enum class ShaderResourceStage : u8
    {
        Vertex,
        Fragment
    };

    enum class ShaderResourceTopology : u8
    {
        TriangleList,
        TriangleStrip,
        LineList
    };

    enum class ShaderResourceFormat : u8
    {
        Undefined,
        R32_UINT,
        R32_SFLOAT,
        R32G32_SFLOAT,
        R32G32B32_SFLOAT,
        R32G32B32A32_SFLOAT
    };

    enum class ShaderResourceDescriptorType : u8
    {
        Uniform,
        Storage,
        CombinedImageSampler
    };

    enum class ShaderResourceBlendOp : u8
    {
        Add
    };

    enum class ShaderResourceBlendFactor : u8
    {
        One,
        SrcAlpha,
        OneMinusSrcAlpha
    };

    struct ShaderResourceBindingData
    {
            u32 binding;
            u32 count;
            u64 block_size_bytes;
            b8 variable_count;
            b8 unbounded;
            str name;
            ShaderResourceDescriptorType descriptor_type;
    };

    struct ShaderResourceDescriptorData
    {
            u32 set;
            std::vector<ShaderResourceBindingData> bindings;
    };

    struct ShaderResourceModuleData
    {
            str stage;
            std::vector<ShaderResourceDescriptorData> descriptors;
            std::vector<u8> code;
    };

    struct ShaderResourceVertexInputData
    {
            ShaderResourceFormat format;
            u32 location;
            u32 size;
            u32 offset;
    };

    struct ShaderResourceColorBlend
    {
            b8 blend_enable;
            ShaderResourceBlendOp color_blend_op;
            ShaderResourceBlendFactor src_color_blend_factor;
            ShaderResourceBlendFactor dst_color_blend_factor;
            ShaderResourceBlendOp alpha_blend_op;
            ShaderResourceBlendFactor src_alpha_blend_factor;
            ShaderResourceBlendFactor dst_alpha_blend_factor;
    };

    struct ShaderResource : public IResource
    {
            str glsl_file_path;
            ShaderResourceTopology topology;
            ShaderResourceColorBlend color_blend;
            std::vector<ShaderResourceVertexInputData> vertex_inputs;
            std::unordered_map<ShaderResourceStage, ShaderResourceModuleData> stages;
    };

    namespace resource
    {
        class ShaderLoader : public IResourceLoader
        {
            public:
                ShaderLoader();
                ~ShaderLoader() override;

                IResource* load_sync(const str& file_path) override;
        };

        MAG_API b8 compile_shader(const str& file_path);
    };  // namespace resource
};  // namespace mag
