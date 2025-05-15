// this header on top
#include "resources/resource_loader.hpp"
// this header on top

#include "core/buffer.hpp"
#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "resources/shader.hpp"
#include "spirv_reflect.h"

namespace mag
{
    namespace resource
    {
        struct ShaderStageData
        {
                str extension;
                ShaderResourceStage stage;
        };

        static const std::map<str, ShaderStageData> shader_stage_map = {
            {"Vertex", {.extension = ".vert", .stage = ShaderResourceStage::Vertex}},
            {"Fragment", {.extension = ".frag", .stage = ShaderResourceStage::Fragment}}};

        static const std::map<str, ShaderResourceTopology> topology_map = {
            {"TriangleList", ShaderResourceTopology::TriangleList},
            {"TriangleStrip", ShaderResourceTopology::TriangleStrip}};

        static const std::map<SpvReflectDescriptorType, ShaderResourceDescriptorType> descriptor_type_map = {
            {SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ShaderResourceDescriptorType::Uniform},
            {SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER, ShaderResourceDescriptorType::Storage},
            {SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ShaderResourceDescriptorType::CombinedImageSampler}};

        b8 load(const str& file_path, ShaderResource* shader)
        {
            fs::json data;

            if (!fs::read_json_data(file_path, data))
            {
                LOG_ERROR("Failed to load shader file: '{0}'", file_path);
                return false;
            }

            b8 incomplete = false;
            std::vector<str> mandatory_params = {"Name", "Stages", "File", "Topology"};
            for (const str& param : mandatory_params)
            {
                if (!data.contains(param))
                {
                    LOG_ERROR("Shader file '{0}' has incomplete fields. Missing: '{1}' field", file_path, param);
                    incomplete = true;
                }
            }

            if (incomplete)
            {
                return false;
            }

            const str name = data["Name"];
            const str glsl_file_path = data["File"];
            const std::vector<str> stages = data["Stages"].get<std::vector<str>>();
            const str topology = data["Topology"];

            if (!topology_map.contains(topology))
            {
                LOG_ERROR("Invalid topology: {0}", topology);
                return false;
            }

            shader->topology = topology_map.at(topology);

            for (const str& stage : stages)
            {
                // Build the binary name from the glsl name
                const str extension = shader_stage_map.at(stage).extension;
                const str binary_file_path = MAG_BUILD_SHADER_NAME(fs::get_file_name(glsl_file_path) + extension);

                Buffer buffer;
                const b8 result = fs::read_binary_data(binary_file_path, buffer);

                if (!result)
                {
                    LOG_ERROR("Failed to load native model binary file: '{0}'", binary_file_path);
                    return false;
                }

                const ShaderResourceStage shader_stage = shader_stage_map.at(stage).stage;
                shader->stages[shader_stage].code = buffer.data;

                SpvReflectShaderModule spv_module;

                // Generate reflection data for a shader
                const SpvReflectResult spv_result =
                    spvReflectCreateShaderModule(buffer.get_size(), buffer.data.data(), &spv_module);

                if (spv_result != SPV_REFLECT_RESULT_SUCCESS)
                {
                    LOG_ERROR("Failed to load shader module reflection: {0}", binary_file_path);
                    return false;
                }

                for (u32 i = 0; i < spv_module.descriptor_set_count; i++)
                {
                    const SpvReflectDescriptorSet spv_descriptor_set = spv_module.descriptor_sets[i];

                    ShaderResourceDescriptorData descriptor = {};
                    descriptor.set = spv_descriptor_set.set;

                    for (u32 j = 0; j < spv_descriptor_set.binding_count; j++)
                    {
                        const SpvReflectDescriptorBinding* spv_binding = spv_descriptor_set.bindings[j];

                        ShaderResourceBindingData binding = {};
                        binding.binding = spv_binding->binding;
                        binding.count = spv_binding->count;
                        binding.descriptor_type = descriptor_type_map.at(spv_binding->descriptor_type);

                        descriptor.bindings.push_back(binding);
                    }

                    shader->stages[shader_stage].descriptors.push_back(descriptor);
                }
            }

            shader->name = name;
            shader->glsl_file_path = glsl_file_path;

            LOG_SUCCESS("Loaded shader: {0}", file_path);
            return true;
        }
    };  // namespace resource
};      // namespace mag
