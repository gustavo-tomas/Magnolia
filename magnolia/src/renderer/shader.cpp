#include "renderer/shader.hpp"

#include <fstream>

#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    // Descriptor Buffer Limits
    // By limiting the number of models/textures we can initialize the descriptor buffers only once and simply rebuild
    // get the descriptor sets again when we modify the uniform/texture data. @TODO: we may want to extend this approach
    // and first check if the descriptor buffer supports the size and also make the limits configurable.
    const u32 MAX_NUMBER_OF_GLOBALS = 1;      // Only one global buffer
    const u32 MAX_NUMBER_OF_INSTANCES = 999;  // The rest is instance buffers
    const u32 MAX_NUMBER_OF_UNIFORMS = MAX_NUMBER_OF_GLOBALS + MAX_NUMBER_OF_INSTANCES;
    const u32 MAX_NUMBER_OF_TEXTURES = 1000;

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

        ugly_init();
    }

    Shader::~Shader()
    {
        if (uniform_inited)
        {
            for (auto& descriptor : uniform_descriptors)
            {
                descriptor.buffer.unmap_memory();
                descriptor.buffer.shutdown();
            }
        }

        if (image_inited)
        {
            for (auto& descriptor : image_descriptors)
            {
                descriptor.buffer.unmap_memory();
                descriptor.buffer.shutdown();
            }
        }

        for (auto& buffers : data_buffers)
        {
            for (auto& buffer : buffers)
            {
                buffer.shutdown();
            }
        }
    }

    void Shader::ugly_init()
    {
        const u32 frame_count = get_context().get_frame_count();

        uniform_descriptors.resize(frame_count);
        image_descriptors.resize(frame_count);
        data_buffers.resize(frame_count);

        // Create descriptors
        for (u64 i = 0; i < frame_count; i++)
        {
            // @TODO: hardcoded values
            if (modules[0]->get_reflection().descriptor_set_count > 0)
            {
                uniform_inited = true;

                uniform_descriptors[i] = DescriptorBuilder::build_layout(modules[0]->get_reflection(), 0);

                uniform_descriptors[i].buffer.initialize(
                    MAX_NUMBER_OF_UNIFORMS * uniform_descriptors[i].size,
                    VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                    VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

                uniform_descriptors[i].buffer.map_memory();

                if (i == 0)  // Only insert once
                {
                    descriptor_set_layouts.push_back(uniform_descriptors[i].layout);  // Global
                    descriptor_set_layouts.push_back(uniform_descriptors[i].layout);  // Instance
                }
            }

            if (modules[1]->get_reflection().descriptor_set_count > 2)
            {
                image_inited = true;

                image_descriptors[i] = DescriptorBuilder::build_layout(modules[1]->get_reflection(), 2);

                image_descriptors[i].buffer.initialize(
                    MAX_NUMBER_OF_TEXTURES * image_descriptors[i].size,
                    VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT |
                        VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                    VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

                image_descriptors[i].buffer.map_memory();

                if (i == 0)  // Only insert once
                {
                    descriptor_set_layouts.push_back(image_descriptors[i].layout);  // Textures
                }
            }

            // We can also initialize the data buffers here
            for (u64 b = 0; b < MAX_NUMBER_OF_UNIFORMS; b++)
            {
                const u64 buffer_size = b < MAX_NUMBER_OF_GLOBALS ? uniforms_map["u_global"].data.size()
                                                                  : uniforms_map["u_instance"].data.size();

                Buffer buffer;
                buffer.initialize(buffer_size,
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                  VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

                data_buffers[i].push_back(buffer);
            }

            DescriptorBuilder::build(uniform_descriptors[i], data_buffers[i]);
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
        data_buffers[curr_frame_number][buffer_index].copy(uniform.data.data(), uniform.data.size());
    }

    void Shader::set_uniform_global(const str& name, const void* data) { set_uniform("u_global", name, data, 0); }

    void Shader::set_uniform_instance(const str& name, const void* data, const u32 instance)
    {
        set_uniform("u_instance", name, data, instance + MAX_NUMBER_OF_GLOBALS);
    }

    void Shader::bind()
    {
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        const u32 curr_frame_number = context.get_curr_frame_number();

        // The pipeline layout should be the same for both pipelines
        std::vector<vk::DescriptorBufferBindingInfoEXT> descriptor_buffer_binding_infos;

        if (uniform_inited)
        {
            descriptor_buffer_binding_infos.push_back(
                {uniform_descriptors[curr_frame_number].buffer.get_device_address(),
                 vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT});
        }

        if (image_inited)
        {
            descriptor_buffer_binding_infos.push_back({image_descriptors[curr_frame_number].buffer.get_device_address(),
                                                       vk::BufferUsageFlagBits::eResourceDescriptorBufferEXT |
                                                           vk::BufferUsageFlagBits::eSamplerDescriptorBufferEXT});
        }

        // Bind descriptor buffers and set offsets
        command_buffer.get_handle().bindDescriptorBuffersEXT(descriptor_buffer_binding_infos);
    }

    void Shader::set_descriptor_buffer_offset(const vk::PipelineLayout& pipeline_layout, const u32 first_set,
                                              const u32 buffer_indices, const u64 buffer_offsets)
    {
        auto& command_buffer = get_context().get_curr_frame().command_buffer;

        const auto pipeline_bind_point = vk::PipelineBindPoint::eGraphics;

        command_buffer.get_handle().setDescriptorBufferOffsetsEXT(pipeline_bind_point, pipeline_layout, first_set,
                                                                  buffer_indices, buffer_offsets);
    }

    void Shader::set_offset_global(const vk::PipelineLayout& pipeline_layout)
    {
        set_descriptor_buffer_offset(pipeline_layout, 0, 0, 0);
    }

    void Shader::set_offset_instance(const vk::PipelineLayout& pipeline_layout, const u32 instance)
    {
        const u32 curr_frame_number = get_context().get_curr_frame_number();
        const u64 buffer_offsets = (instance + 1) * uniform_descriptors[curr_frame_number].size;

        set_descriptor_buffer_offset(pipeline_layout, 1, 0, buffer_offsets);
    }

    void Shader::set_offset_material(const vk::PipelineLayout& pipeline_layout, const u32 index)
    {
        const u32 curr_frame_number = get_context().get_curr_frame_number();
        const u64 buffer_offsets = index * image_descriptors[curr_frame_number].size;

        set_descriptor_buffer_offset(pipeline_layout, 2, 1, buffer_offsets);
    }

    void Shader::add_model(const Model& model)
    {
        auto& context = get_context();

        const u32 frame_count = context.get_frame_count();

        // Put all models textures in a single array
        for (auto& mesh : model.meshes)
        {
            for (auto& texture : mesh.textures)
            {
                textures.emplace_back(texture);
            }
        }

        // Create descriptor buffer that holds texture data
        for (u32 i = 0; i < frame_count; i++)
        {
            if (textures.size() + 1 > MAX_NUMBER_OF_TEXTURES)
            {
                LOG_ERROR("Maximum number of textures exceeded: {0}", MAX_NUMBER_OF_TEXTURES);
                return;
            }

            DescriptorBuilder::build(image_descriptors[i], textures);
        }
    }
};  // namespace mag
