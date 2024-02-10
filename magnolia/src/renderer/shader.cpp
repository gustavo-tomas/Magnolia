#include "renderer/shader.hpp"

#include <fstream>

#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "spirv_reflect.h"

#define SPIRV_CHECK(result) ASSERT((result) == SPV_REFLECT_RESULT_SUCCESS, "Spirv check failed")

namespace mag
{
    void Shader::initialize(const str& file, const vk::ShaderStageFlagBits stage)
    {
        auto& context = get_context();
        this->stage = stage;
        this->file = file;

        std::ifstream file_stream(file, std::ios::ate | std::ios::binary);
        ASSERT(file_stream.is_open(), "Failed to open file: " + file);

        const size_t file_size = static_cast<size_t>(file_stream.tellg());
        std::vector<u32> buffer(file_size / sizeof(u32));
        file_stream.seekg(0);
        file_stream.read(reinterpret_cast<char*>(buffer.data()), file_size);
        file_stream.close();

        this->module = context.get_device().createShaderModule(vk::ShaderModuleCreateInfo({}, buffer));

        // @TODO: testing
        // Generate reflection data for a shader
        {
            SpvReflectShaderModule spv_module;
            SpvReflectResult result = spvReflectCreateShaderModule(file_size, buffer.data(), &spv_module);
            SPIRV_CHECK(result);

            // Enumerate and extract shader's input variables
            uint32_t var_count = 0;
            result = spvReflectEnumerateInputVariables(&spv_module, &var_count, NULL);
            SPIRV_CHECK(result);

            SpvReflectInterfaceVariable** input_vars =
                static_cast<SpvReflectInterfaceVariable**>(malloc(var_count * sizeof(SpvReflectInterfaceVariable*)));
            result = spvReflectEnumerateInputVariables(&spv_module, &var_count, input_vars);
            SPIRV_CHECK(result);

            /* ... */

            spvReflectDestroyShaderModule(&spv_module);
        }
    }

    void Shader::shutdown()
    {
        auto& context = get_context();
        context.get_device().destroyShaderModule(module);
    }
};  // namespace mag
