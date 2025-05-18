// this header on top
#include "resources/resource_loader.hpp"
// this header on top

#include "core/buffer.hpp"
#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "resources/shader.hpp"
#include "spirv_reflect.h"

// @TODO: temp
#include "../magnolia/assets/shaders/include/common.h"

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
            {"Fragment", {.extension = ".frag", .stage = ShaderResourceStage::Fragment}},
        };

        static const std::map<str, ShaderResourceTopology> topology_map = {
            {"TriangleList", ShaderResourceTopology::TriangleList},
            {"TriangleStrip", ShaderResourceTopology::TriangleStrip},
        };

        static const std::map<SpvReflectDescriptorType, ShaderResourceDescriptorType> descriptor_type_map = {
            {SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ShaderResourceDescriptorType::Uniform},
            {SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER, ShaderResourceDescriptorType::Storage},
            {SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ShaderResourceDescriptorType::CombinedImageSampler},
        };

        static const std::map<SpvReflectDescriptorType, u64> descriptor_type_size_map = {
            {SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER, Max_SSBO_Byte_Size},
            {SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Max_Descriptor_Array_Size},
        };

        static const std::map<SpvReflectFormat, ShaderResourceFormat> format_type_map = {
            {SPV_REFLECT_FORMAT_UNDEFINED, ShaderResourceFormat::Undefined},
            {SPV_REFLECT_FORMAT_R32_UINT, ShaderResourceFormat::R32_UINT},
            {SPV_REFLECT_FORMAT_R32G32_SFLOAT, ShaderResourceFormat::R32G32_SFLOAT},
            {SPV_REFLECT_FORMAT_R32G32B32_SFLOAT, ShaderResourceFormat::R32G32B32_SFLOAT},
            {SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT, ShaderResourceFormat::R32G32B32A32_SFLOAT},
        };

        static u64 get_aligned_size(const u64 original_size, const u64 alignment)
        {
            return (original_size + alignment - 1) & ~(alignment - 1);
        }

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

                // Descriptor sets
                for (u32 i = 0; i < spv_module.descriptor_set_count; i++)
                {
                    const SpvReflectDescriptorSet spv_descriptor_set = spv_module.descriptor_sets[i];

                    ShaderResourceDescriptorData descriptor = {};
                    descriptor.set = spv_descriptor_set.set;

                    for (u32 j = 0; j < spv_descriptor_set.binding_count; j++)
                    {
                        const SpvReflectDescriptorBinding* spv_binding = spv_descriptor_set.bindings[j];

                        u64 block_size = 0;
                        for (u32 k = 0; k < spv_binding->block.member_count; k++)
                        {
                            // @TODO: block padded_size is buggy, so we use this function to calculated the aligned size
                            block_size += get_aligned_size(spv_binding->block.members[k].size, 16);
                        }

                        ShaderResourceBindingData binding = {};
                        binding.binding = spv_binding->binding;
                        binding.count = spv_binding->count;
                        binding.block_size = block_size;
                        binding.name = spv_binding->name;
                        binding.descriptor_type = descriptor_type_map.at(spv_binding->descriptor_type);

                        // Uniform buffer has a fixed size
                        if (spv_binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                        {
                            binding.max_size = block_size;
                        }

                        // Arrays have other limits
                        else
                        {
                            binding.max_size = descriptor_type_size_map.at(spv_binding->descriptor_type);
                        }

                        descriptor.bindings.push_back(binding);
                    }

                    shader->stages[shader_stage].descriptors.push_back(descriptor);
                }

                // Vertex input attributes
                if (shader_stage == ShaderResourceStage::Vertex)
                {
                    // Add vertex attributes sorted by location
                    std::map<u32, const SpvReflectInterfaceVariable*> sorted_input_variables;
                    for (u32 i = 0; i < spv_module.input_variable_count; i++)
                    {
                        const SpvReflectInterfaceVariable* const variable = spv_module.input_variables[i];

                        // Filter built-in variables
                        if (variable->location < Max_U32)
                        {
                            sorted_input_variables[variable->location] = variable;
                        }
                    }

                    u32 offset = 0;
                    for (const auto& [location, variable] : sorted_input_variables)
                    {
                        u32 size = variable->numeric.scalar.width / 8;
                        size *=
                            variable->numeric.vector.component_count > 0 ? variable->numeric.vector.component_count : 1;

                        ShaderResourceVertexInputData vertex_input = {};
                        vertex_input.format = format_type_map.at(variable->format);
                        vertex_input.offset = offset;
                        vertex_input.location = location;
                        vertex_input.size = size;

                        shader->vertex_inputs.push_back(vertex_input);

                        offset += size;
                    }
                }
            }

            shader->name = name;
            shader->glsl_file_path = glsl_file_path;

            LOG_SUCCESS("Loaded shader: {0}", file_path);
            return true;
        }
    };  // namespace resource
};      // namespace mag
