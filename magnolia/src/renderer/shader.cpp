#include "renderer/shader.hpp"

#include <fstream>

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    void Shader::create(const str& file, const vk::ShaderStageFlagBits stage)
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
    }

    void Shader::destroy()
    {
        auto& context = get_context();
        context.get_device().destroyShaderModule(module);
    }
};  // namespace mag
