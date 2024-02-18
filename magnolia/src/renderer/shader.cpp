#include "renderer/shader.hpp"

#include <fstream>

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    void Shader::initialize(const str& file)
    {
        auto& context = get_context();
        this->file = file;

        std::ifstream file_stream(file, std::ios::ate | std::ios::binary);
        ASSERT(file_stream.is_open(), "Failed to open file: " + file);

        const size_t file_size = static_cast<size_t>(file_stream.tellg());
        std::vector<u32> buffer(file_size / sizeof(u32));
        file_stream.seekg(0);
        file_stream.read(reinterpret_cast<char*>(buffer.data()), file_size);
        file_stream.close();

        this->module = context.get_device().createShaderModule(vk::ShaderModuleCreateInfo({}, buffer));

        // Generate reflection data for a shader
        SpvReflectResult result = spvReflectCreateShaderModule(file_size, buffer.data(), &spv_module);
        VK_CHECK(VK_CAST(result));

        // Enumerate and extract shader's input variables
        u32 var_count = 0;
        result = spvReflectEnumerateInputVariables(&spv_module, &var_count, NULL);
        VK_CHECK(VK_CAST(result));

        SpvReflectInterfaceVariable** input_vars =
            static_cast<SpvReflectInterfaceVariable**>(malloc(var_count * sizeof(SpvReflectInterfaceVariable*)));
        result = spvReflectEnumerateInputVariables(&spv_module, &var_count, input_vars);
        VK_CHECK(VK_CAST(result));

        // Get reflection data
        for (u32 s = 0; s < spv_module.descriptor_set_count; s++)
        {
            DescriptorSet descriptor_set = {};
            descriptor_set.set = s;

            for (u32 b = 0; b < spv_module.descriptor_sets[s].binding_count; b++)
            {
                auto& current_binding = spv_module.descriptor_sets[s].bindings[b];

                Binding binding = {};
                binding.binding = current_binding->binding;
                binding.descriptor_type = static_cast<vk::DescriptorType>(current_binding->descriptor_type);

                descriptor_set.bindings.push_back(binding);
            }

            spv_reflection.descriptor_sets.push_back(descriptor_set);
        }

        spv_reflection.shader_stage = static_cast<vk::ShaderStageFlagBits>(spv_module.shader_stage);
    }

    void Shader::shutdown()
    {
        auto& context = get_context();
        context.get_device().destroyShaderModule(module);
        spvReflectDestroyShaderModule(&spv_module);
    }
};  // namespace mag
