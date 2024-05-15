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
        // Gather all shader stages from the modules
        for (const auto& module : modules)
        {
            stages |= static_cast<vk::ShaderStageFlagBits>(module->get_reflection().shader_stage);
        }
    };

    // We can automate some of these steps with spv reflect but it is better to set this values manually
    void Shader::add_attribute(const vk::Format format, const u32 size, const u32 offset)
    {
        const vk::VertexInputAttributeDescription attribute(location++, 0, format, offset);
        attributes.push_back(attribute);

        // Rewrite the binding when an attribute is added
        stride += size;
        binding = vk::VertexInputBindingDescription(0, stride, vk::VertexInputRate::eVertex);
    }

    void Shader::add_uniform(const str& name)
    {
        // Check all modules for the desired uniform
        for (const auto& module : modules)
        {
            // Check for the binding with the same name
            const auto& reflection = module->get_reflection();
            for (u32 b = 0; b < reflection.descriptor_binding_count; b++)
            {
                const auto& binding = reflection.descriptor_bindings[b];
                if (std::strcmp(binding.name, name.c_str()) == 0)
                {
                    add_binding(binding.set, binding, stages);
                    build_layout(binding.set);
                    return;
                }
            }
        }

        ASSERT(false, "No uniform with name: '" + name + "' was found in this shader");
    }

    void Shader::add_binding(const u32 set, const SpvReflectDescriptorBinding& descriptor_binding,
                             const vk::ShaderStageFlags stage)
    {
        // Create the layout binding for the specified set
        const u32 binding = descriptor_binding.binding;
        const u32 count = descriptor_binding.count;
        const auto type = static_cast<vk::DescriptorType>(descriptor_binding.descriptor_type);

        const vk::DescriptorSetLayoutBinding layout_binding(binding, type, count, stage);
        layout_bindings[set].push_back(layout_binding);
    }

    void Shader::build_layout(const u32 set)
    {
        auto& context = get_context();
        auto& device = context.get_device();
        auto& cache = context.get_descriptor_cache();

        for (const auto& bindings : layout_bindings[set])
        {
            Descriptor descriptor;

            // Build layout
            const vk::DescriptorSetLayoutCreateInfo layout_info(
                vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT, bindings);
            descriptor.layout = cache.create_descriptor_layout(&layout_info);

            auto descriptor_buffer_properties = context.get_descriptor_buffer_properties();

            // Get set layout descriptor sizes and adjust them to satisfy alignment requirements.
            const u64 alignment = descriptor_buffer_properties.descriptorBufferOffsetAlignment;
            descriptor.size = device.getDescriptorSetLayoutSizeEXT(descriptor.layout);
            descriptor.size = (descriptor.size + alignment - 1) & ~(alignment - 1);

            // Get descriptor bindings offsets as descriptors are placed inside set layout by those offsets.
            // @TODO: The binding can be non zero depending on the driver:
            // https://github.com/KhronosGroup/Vulkan-Samples/tree/main/samples/extensions/descriptor_buffer_basic
            descriptor.offset = device.getDescriptorSetLayoutBindingOffsetEXT(descriptor.layout, 0);

            descriptors[set] = descriptor;
        }
    }
};  // namespace mag
