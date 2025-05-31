#pragma once

#include <map>
#include <vector>

#include "core/types.hpp"
#include "resources/resource.hpp"

namespace mag
{
    enum class ShaderResourceStage
    {
        Vertex,
        Fragment
    };

    enum class ShaderResourceTopology
    {
        TriangleList,
        TriangleStrip,
        LineList
    };

    enum class ShaderResourceFormat
    {
        Undefined,
        R32_UINT,
        R32G32_SFLOAT,
        R32G32B32_SFLOAT,
        R32G32B32A32_SFLOAT
    };

    enum class ShaderResourceDescriptorType
    {
        Uniform,
        Storage,
        CombinedImageSampler
    };

    enum class ShaderResourceBlendOp
    {
        Add
    };

    enum class ShaderResourceBlendFactor
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
            str name = "";
            str glsl_file_path = "";
            ShaderResourceTopology topology;
            ShaderResourceColorBlend color_blend;
            std::vector<ShaderResourceVertexInputData> vertex_inputs;
            std::map<ShaderResourceStage, ShaderResourceModuleData> stages;
    };

    namespace resource
    {
        ref<ShaderResource> MAG_API get_shader(const str& name);
    };  // namespace resource
};      // namespace mag
