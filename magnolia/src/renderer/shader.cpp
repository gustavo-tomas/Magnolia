#include "renderer/shader.hpp"

#include <fstream>

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    std::shared_ptr<Shader> ShaderLoader::load(const str& name, const str& vertex_file, const str& fragment_file)
    {
        auto it = shaders.find(name);
        if (it != shaders.end()) return it->second;

        const auto vertex_module = load_module(vertex_file);
        const auto fragment_module = load_module(fragment_file);

        Shader* shader = new Shader({vertex_module, fragment_module});

        LOG_SUCCESS("Loaded shader: {0}", name);
        shaders[name] = std::shared_ptr<Shader>(shader);
        return shaders[name];
    }

    std::shared_ptr<ShaderModule> ShaderLoader::load_module(const str& file)
    {
        auto it = shader_modules.find(file);
        if (it != shader_modules.end()) return it->second;

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

        ShaderModule* shader_module = new ShaderModule(file, module, spv_module);

        LOG_INFO("Loaded shader module: {0}", file);
        shader_modules[file] = std::shared_ptr<ShaderModule>(shader_module);
        return shader_modules[file];
    }

    ShaderLoader::~ShaderLoader()
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        for (const auto& shader_pair : shader_modules)
        {
            const auto& shader = shader_pair.second;

            context.get_device().destroyShaderModule(shader->get_handle());
            spvReflectDestroyShaderModule(const_cast<SpvReflectShaderModule*>(&shader->get_reflection()));
        }
    }

    Shader::Shader(const std::vector<std::shared_ptr<ShaderModule>>& modules) : modules(modules)
    {
        // Initialize all uniforms
        for (const auto& module : modules)
        {
            const auto& reflection = module->get_reflection();
            for (u32 i = 0; i < reflection.descriptor_binding_count; i++)
            {
                auto& descriptor_binding = reflection.descriptor_bindings[i];

                // Already initialized
                if (uniforms_map.contains(descriptor_binding.name)) continue;

                uniforms_map[descriptor_binding.name].descriptor_binding = descriptor_binding;
                uniforms_map[descriptor_binding.name].data.resize(descriptor_binding.block.size);
            }
        }
    }

    // We can automate some of these steps with spv reflect but it is better to set this values manually
    void Shader::add_attribute(const vk::Format format, const u32 size, const u32 offset)
    {
        vk::VertexInputAttributeDescription attribute(location++, 0, format, offset);
        vertex_attributes.push_back(attribute);

        // Rewrite the binding when an attribute is added
        stride += size;

        vertex_bindings.clear();
        vertex_bindings.push_back(vk::VertexInputBindingDescription(0, stride, vk::VertexInputRate::eVertex));
    }

    void Shader::set_uniform(const str& scope, const str& name, const void* data, const u32 buffer_index)
    {
        auto& context = get_context();
        auto& descriptor_cache = context.get_descriptor_cache();
        const u32 curr_frame_number = context.get_curr_frame_number();

        auto& uniform = uniforms_map[scope];
        auto& block = uniform.descriptor_binding.block;

        u64 offset = 0;
        u64 size = 0;
        for (u32 i = 0; i < block.member_count; i++)
        {
            const auto& member = block.members[i];
            if (strcmp(member.name, name.c_str()) == 0)
            {
                offset = member.offset;
                size = member.size;
                break;
            }
        }

        memcpy(uniform.data.data() + offset, data, size);
        descriptor_cache.get_data_buffers()[curr_frame_number][buffer_index].copy(uniform.data.data(),
                                                                                  uniform.data.size());
    }

    void Shader::set_uniform_global(const str& name, const void* data) { set_uniform("u_global", name, data, 0); }

    void Shader::set_uniform_instance(const str& name, const void* data, const u32 instance)
    {
        // @TODO: hardcoded number of global uniforms (1)
        set_uniform("u_instance", name, data, instance + 1);
    }
};  // namespace mag
