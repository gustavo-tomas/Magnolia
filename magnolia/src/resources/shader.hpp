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
        TriangleStrip
    };

    enum class ShaderResourceDescriptorType
    {
        Uniform,
        Storage,
        CombinedImageSampler
    };

    struct ShaderResourceBindingData
    {
            u32 binding;
            u32 count;
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

    struct ShaderResource : public IResource
    {
            str name = "";
            str glsl_file_path = "";
            ShaderResourceTopology topology;
            std::map<ShaderResourceStage, ShaderResourceModuleData> stages;
    };

    namespace resource
    {
        ref<ShaderResource> get_shader(const str& name);
    };  // namespace resource
};      // namespace mag
