#include "renderer/shader.hpp"

#include <fstream>

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    std::shared_ptr<Shader> ShaderLoader::load(const str& file)
    {
        auto it = shaders.find(file);
        if (it != shaders.end()) return it->second;

        auto& context = get_context();

        std::ifstream file_stream(file, std::ios::ate | std::ios::binary);
        ASSERT(file_stream.is_open(), "Failed to open file: " + file);

        const size_t file_size = static_cast<size_t>(file_stream.tellg());
        std::vector<u32> buffer(file_size / sizeof(u32));
        file_stream.seekg(0);
        file_stream.read(reinterpret_cast<char*>(buffer.data()), file_size);
        file_stream.close();

        const vk::ShaderModule module = context.get_device().createShaderModule(vk::ShaderModuleCreateInfo({}, buffer));

        // Generate reflection data for a shader
        SpvReflectShaderModule spv_module = {};
        SpvReflectResult result = spvReflectCreateShaderModule(file_size, buffer.data(), &spv_module);
        VK_CHECK(VK_CAST(result));

        Shader* shader = new Shader(file, module, spv_module);

        LOG_SUCCESS("Loaded shader: {0}", file);
        shaders[file] = std::shared_ptr<Shader>(shader);
        return shaders[file];
    }

    ShaderLoader::~ShaderLoader()
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        for (const auto& shader_pair : shaders)
        {
            const auto& shader = shader_pair.second;

            context.get_device().destroyShaderModule(shader->get_handle());
            spvReflectDestroyShaderModule(const_cast<SpvReflectShaderModule*>(&shader->get_reflection()));
        }
    }

    // We can automate some of these steps with spv reflect but it is better to set this values manually
    void Shader::add_attribute(const vk::Format format, const u32 size, const u32 offset)
    {
        vk::VertexInputAttributeDescription attribute(location++, 0, format, offset);
        attributes.push_back(attribute);

        // Rewrite the binding when an attribute is added
        stride += size;
        binding = vk::VertexInputBindingDescription(0, stride, vk::VertexInputRate::eVertex);
    }
};  // namespace mag
