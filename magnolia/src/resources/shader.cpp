#include "magnolia/resources/shader.hpp"

#include "magnolia/core/buffer.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/platform/file_system.hpp"
#include "magnolia/platform/json.hpp"
#include "spirv_reflect.h"

namespace mag
{
    namespace resource
    {
        struct ShaderStageData
        {
                str extension;
                str define;
                str stage_str;
                ShaderResourceStage stage;
        };

        static const std::unordered_map<str, ShaderStageData> shader_stage_map = {
            {"Vertex",
             {.extension = ".vert",
              .define = "VERTEX_SHADER",
              .stage_str = "vertex",
              .stage = ShaderResourceStage::Vertex}},
            {"Fragment",
             {.extension = ".frag",
              .define = "FRAGMENT_SHADER",
              .stage_str = "fragment",
              .stage = ShaderResourceStage::Fragment}},
        };

        static const std::unordered_map<str, ShaderResourceTopology> topology_map = {
            {"TriangleList", ShaderResourceTopology::TriangleList},
            {"TriangleStrip", ShaderResourceTopology::TriangleStrip},
            {"LineList", ShaderResourceTopology::LineList},
        };

        static const std::unordered_map<SpvReflectDescriptorType, ShaderResourceDescriptorType> descriptor_type_map = {
            {SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ShaderResourceDescriptorType::Uniform},
            {SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER, ShaderResourceDescriptorType::Storage},
            {SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ShaderResourceDescriptorType::CombinedImageSampler},
        };

        static const std::unordered_map<SpvReflectFormat, ShaderResourceFormat> format_type_map = {
            {SPV_REFLECT_FORMAT_UNDEFINED, ShaderResourceFormat::Undefined},
            {SPV_REFLECT_FORMAT_R32_UINT, ShaderResourceFormat::R32_UINT},
            {SPV_REFLECT_FORMAT_R32_SFLOAT, ShaderResourceFormat::R32_SFLOAT},
            {SPV_REFLECT_FORMAT_R32G32_SFLOAT, ShaderResourceFormat::R32G32_SFLOAT},
            {SPV_REFLECT_FORMAT_R32G32B32_SFLOAT, ShaderResourceFormat::R32G32B32_SFLOAT},
            {SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT, ShaderResourceFormat::R32G32B32A32_SFLOAT},
        };

        static const std::unordered_map<str, ShaderResourceBlendOp> blend_op_map = {
            {"Add", ShaderResourceBlendOp::Add},
        };

        static const std::unordered_map<str, ShaderResourceBlendFactor> blend_factor_map = {
            {"One", ShaderResourceBlendFactor::One},
            {"SrcAlpha", ShaderResourceBlendFactor::SrcAlpha},
            {"OneMinusSrcAlpha", ShaderResourceBlendFactor::OneMinusSrcAlpha},
        };

        static b8 compile_shader_submodule(const str& input_file_path, const str& output_file_path,
                                           const std::vector<str>& include_paths, const std::vector<str>& defines,
                                           const str& shader_stage);

        static void read_descriptor_sets(const SpvReflectShaderModule& spv_module, ShaderResourceStage shader_stage,
                                         ShaderResource* resource);

        static void read_vertex_input_attributes(const SpvReflectShaderModule& spv_module, ShaderResource* resource);

        static u64 get_aligned_size(const u64 original_size, const u64 alignment)
        {
            return (original_size + alignment - 1) & ~(alignment - 1);
        }

        static b8 load_shader_description(const str& file_path, ShaderResource* shader)
        {
            fs::json data;

            if (!fs::read_json_data(file_path, data))
            {
                LOG_ERROR("Failed to load shader file: '{0}'", file_path);
                return false;
            }

            b8 incomplete = false;
            const std::vector<str> mandatory_params = {"Name", "Stages", "File", "Topology"};
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

            if (data.contains("ColorBlendOp") || data.contains("AlphaBlendOp"))
            {
                shader->color_blend.blend_enable = true;
                shader->color_blend.color_blend_op = blend_op_map.at(data["ColorBlendOp"]);
                shader->color_blend.alpha_blend_op = blend_op_map.at(data["AlphaBlendOp"]);
                shader->color_blend.src_color_blend_factor = blend_factor_map.at(data["SrcColorBlendFactor"]);
                shader->color_blend.dst_color_blend_factor = blend_factor_map.at(data["DstColorBlendFactor"]);
                shader->color_blend.src_alpha_blend_factor = blend_factor_map.at(data["SrcAlphaBlendFactor"]);
                shader->color_blend.dst_alpha_blend_factor = blend_factor_map.at(data["DstAlphaBlendFactor"]);
            }

            else
            {
                shader->color_blend.blend_enable = false;
            }

            for (const str& stage : stages)
            {
                const ShaderResourceStage shader_stage = shader_stage_map.at(stage).stage;
                shader->stages[shader_stage].stage = stage;
            }

            shader->name = name;
            shader->file_path = file_path;
            shader->glsl_file_path = glsl_file_path;
            shader->topology = topology_map.at(topology);

            return true;
        }

        void read_descriptor_sets(const SpvReflectShaderModule& spv_module, const ShaderResourceStage shader_stage,
                                  ShaderResource* resource)
        {
            const std::vector<SpvReflectDescriptorSet> spv_descriptor_sets(
                &spv_module.descriptor_sets[0], &spv_module.descriptor_sets[0] + spv_module.descriptor_set_count);

            for (const SpvReflectDescriptorSet& spv_descriptor_set : spv_descriptor_sets)
            {
                ShaderResourceDescriptorData descriptor = {};
                descriptor.set = spv_descriptor_set.set;

                for (u32 j = 0; j < spv_descriptor_set.binding_count; j++)
                {
                    const SpvReflectDescriptorBinding* spv_binding = spv_descriptor_set.bindings[j];

                    u64 block_size = 0;
                    for (u32 k = 0; k < spv_binding->block.member_count; k++)
                    {
                        // @TODO: block padded_size is buggy, so we use this function to calculated the aligned size
                        // https://github.com/KhronosGroup/SPIRV-Reflect/issues/280
                        const u64 alignment = 16;
                        block_size += get_aligned_size(spv_binding->block.members[k].size, alignment);
                    }

                    ShaderResourceBindingData binding = {};
                    binding.binding = spv_binding->binding;
                    binding.count = spv_binding->count;
                    binding.block_size_bytes = block_size;
                    binding.name = spv_binding->name;
                    binding.variable_count = false;
                    binding.unbounded = false;
                    binding.descriptor_type = descriptor_type_map.at(spv_binding->descriptor_type);

                    // Set the correct values for descriptor count and max binding size.
                    // We need to to this because spv is a little confused and can't process arrays
                    // correctly. Unbounded arrays count will be decided on the gfx side.

                    // Assume that arrays with count = 1 are variable count (@TODO: this is just a fix)
                    const b8 is_unbounded_array = spv_binding->array.dims_count > 0;

                    // Storage buffers are assume unbounded (@TODO: this assumes that bounded SSBO block members are
                    // also unbounded)
                    const b8 is_ssbo = binding.descriptor_type == ShaderResourceDescriptorType::Storage;

                    if (is_unbounded_array || is_ssbo)
                    {
                        binding.unbounded = true;
                    }

                    if (is_unbounded_array && j == spv_descriptor_set.binding_count - 1)
                    {
                        binding.variable_count = true;
                    }

                    descriptor.bindings.push_back(binding);
                }

                resource->stages[shader_stage].descriptors.push_back(descriptor);
            }
        }

        void read_vertex_input_attributes(const SpvReflectShaderModule& spv_module, ShaderResource* resource)
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
                const u32 size =
                    (variable->numeric.scalar.width / 8) *
                    (variable->numeric.vector.component_count > 0 ? variable->numeric.vector.component_count : 1);

                ShaderResourceVertexInputData vertex_input = {};
                vertex_input.format = format_type_map.at(variable->format);
                vertex_input.offset = offset;
                vertex_input.location = location;
                vertex_input.size = size;

                resource->vertex_inputs.push_back(vertex_input);

                offset += size;
            }
        }

        b8 load_sync(const str& file_path, ResourceManager* rm, ShaderResource* resource)
        {
            (void)rm;

            if (!load_shader_description(file_path, resource))
            {
                return false;
            }

            for (const auto& [stage, module_data] : resource->stages)
            {
                // Build the binary name from the glsl name
                const str extension = shader_stage_map.at(module_data.stage).extension;
                const str binary_file_path =
                    MAG_BUILD_SHADER_NAME(fs::get_file_name(resource->glsl_file_path) + extension);

                Buffer buffer;
                const b8 result = fs::read_binary_data(binary_file_path, buffer);

                if (!result)
                {
                    LOG_ERROR("Failed to load native model binary file: '{0}'", binary_file_path);
                    return false;
                }

                const ShaderResourceStage shader_stage = shader_stage_map.at(module_data.stage).stage;
                resource->stages[shader_stage].code = buffer.data;

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
                read_descriptor_sets(spv_module, shader_stage, resource);

                // Vertex input attributes
                if (shader_stage == ShaderResourceStage::Vertex)
                {
                    read_vertex_input_attributes(spv_module, resource);
                }

                // Destroy after use
                spvReflectDestroyShaderModule(&spv_module);
            }

            LOG_SUCCESS("Loaded shader: {0}", file_path);
            return true;
        }

        b8 compile_shader(const str& input_file_path)
        {
            mag::ShaderResource shader_resource = {};

            if (!load_shader_description(input_file_path, &shader_resource))
            {
                LOG_ERROR("Failed to compile shader: '{0}'", input_file_path);
                return false;
            }

            b8 result = true;
            for (const auto& [stage, data] : shader_stage_map)
            {
                if (shader_resource.stages.contains(data.stage))
                {
                    const str extension = shader_stage_map.at(stage).extension;
                    const str binary_file_path =
                        MAG_BUILD_SHADER_NAME(fs::get_file_name(shader_resource.glsl_file_path) + extension);

                    const str& submodule_file_path =
                        fs::path(input_file_path).parent_path() / shader_resource.glsl_file_path;

                    result = result && compile_shader_submodule(submodule_file_path, binary_file_path, {},
                                                                {data.define}, data.stage_str);
                }
            }

            if (result)
            {
                LOG_SUCCESS("Finished compiling shader: '{0}'", input_file_path);
            }

            return result;
        }

        static b8 compile_shader_submodule(const str& input_file_path, const str& output_file_path,
                                           const std::vector<str>& include_paths, const std::vector<str>& defines,
                                           const str& shader_stage)
        {
            LOG_INFO("Compiling shader submodule '{0}' to '{1}'", input_file_path, output_file_path);

            // Create directories if they dont exist
            fs::create_directories(fs::path(output_file_path).parent_path());

            // @TODO: for now no optimizations
            str compile_script_cmd = MAG_EXT_GLSLC " -O0 -g";

            // Include paths
            compile_script_cmd = std::accumulate(include_paths.begin(), include_paths.end(), compile_script_cmd,
                                                 [](const str& cmd, const str& arg) { return cmd + " -I" + arg; });

            // Defines
            compile_script_cmd = std::accumulate(defines.begin(), defines.end(), compile_script_cmd,
                                                 [](const str& cmd, const str& arg) { return cmd + " -D" + arg; });

            // Stage
            compile_script_cmd += " -fshader-stage=" + shader_stage;

            compile_script_cmd += " " + input_file_path + " -o " + output_file_path;

            // Execute
            return system(compile_script_cmd.c_str()) == 0;
        }
    };  // namespace resource
};  // namespace mag
