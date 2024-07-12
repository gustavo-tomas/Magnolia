#include "renderer/shader.hpp"

#include <fstream>

#include "core/application.hpp"
#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "renderer/descriptors.hpp"
#include "renderer/pipeline.hpp"

namespace mag
{
    std::shared_ptr<Shader> ShaderManager::load(const str& name, const str& vertex_file, const str& fragment_file)
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

    std::shared_ptr<ShaderModule> ShaderManager::load_module(const str& file)
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

    ShaderManager::~ShaderManager()
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

                const str scope = descriptor_binding.name;
                const u32 size = descriptor_binding.block.size;
                const vk::DescriptorType type = static_cast<vk::DescriptorType>(descriptor_binding.descriptor_type);

                // Create buffer for ubos
                if (type == vk::DescriptorType::eUniformBuffer)
                {
                    uniforms_map[scope].descriptor_binding = descriptor_binding;
                    uniforms_map[scope].data.resize(size);
                    uniforms_map[scope].buffer.initialize(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                          VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                                                          VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

                    auto& descriptor_set = uniforms_map[scope].descriptor_set;
                    auto& descriptor_set_layout = uniforms_map[scope].descriptor_set_layout;
                    auto& buffer = uniforms_map[scope].buffer;

                    DescriptorBuilder::create_descriptor_for_ubo(descriptor_set, descriptor_set_layout, buffer, size,
                                                                 0);

                    descriptor_set_layouts.push_back(descriptor_set_layout);
                }

                // Create samplers for textures
                else if (type == vk::DescriptorType::eCombinedImageSampler)
                {
                    auto& app = get_application();
                    auto& material_manager = app.get_material_manager();

                    uniforms_map[scope].descriptor_binding = descriptor_binding;

                    auto& descriptor_set = uniforms_map[scope].descriptor_set;
                    auto& descriptor_set_layout = uniforms_map[scope].descriptor_set_layout;

                    auto default_mat = material_manager.get(DEFAULT_MATERIAL_NAME);

                    // Albedo
                    if (descriptor_binding.set == 3)
                    {
                        DescriptorBuilder::create_descriptor_for_texture(descriptor_binding.binding,
                                                                         default_mat->textures[Material::Albedo],
                                                                         descriptor_set, descriptor_set_layout);
                    }

                    // Normal
                    if (descriptor_binding.set == 4)
                    {
                        DescriptorBuilder::create_descriptor_for_texture(descriptor_binding.binding,
                                                                         default_mat->textures[Material::Normal],
                                                                         descriptor_set, descriptor_set_layout);
                    }

                    descriptor_set_layouts.push_back(descriptor_set_layout);
                }
            }
        }
    }

    Shader::~Shader()
    {
        for (auto& [scope, ubo] : uniforms_map)
        {
            ubo.buffer.shutdown();
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

    void Shader::set_uniform(const str& scope, const str& name, const void* data)
    {
        // auto& context = get_context();
        // const u32 curr_frame_number = context.get_curr_frame_number();

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
        uniform.buffer.copy(uniform.data.data(), uniform.data.size());
    }

    void Shader::bind_texture(const Pipeline& pipeline, const str& name, const vk::DescriptorSet& descriptor_set)
    {
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;

        auto& ubo = uniforms_map[name];

        ubo.descriptor_set = descriptor_set;

        // Rebind the texture descriptor
        command_buffer.bind_descriptor_set(vk::PipelineBindPoint::eGraphics, pipeline.get_layout(),
                                           ubo.descriptor_binding.set, ubo.descriptor_set);
    }

    void Shader::bind(const Pipeline& pipeline)
    {
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;

        for (auto& [scope, ubo] : uniforms_map)
        {
            command_buffer.bind_descriptor_set(vk::PipelineBindPoint::eGraphics, pipeline.get_layout(),
                                               ubo.descriptor_binding.set, ubo.descriptor_set);
        }
    }
};  // namespace mag
