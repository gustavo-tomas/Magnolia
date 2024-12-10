#include "renderer/shader.hpp"

#include <vulkan/vulkan.hpp>

#include "core/application.hpp"
#include "core/assert.hpp"
#include "core/logger.hpp"
#include "math/generic.hpp"
#include "renderer/buffers.hpp"
#include "renderer/context.hpp"
#include "renderer/descriptors.hpp"
#include "renderer/frame.hpp"
#include "renderer/pipeline.hpp"
#include "renderer/renderer.hpp"
#include "renderer/renderer_image.hpp"
#include "resources/image.hpp"
#include "resources/material.hpp"
#include "resources/shader_loader.hpp"
#include "spirv_reflect.h"

namespace mag
{
    using namespace mag::math;

    ref<Shader> ShaderManager::get(const str& file_path)
    {
        auto it = shaders.find(file_path);
        if (it != shaders.end()) return it->second;

        auto& app = get_application();
        auto& shader_loader = app.get_shader_loader();

        ShaderConfiguration shader_configuration;

        if (!shader_loader.load(file_path, &shader_configuration))
        {
            LOG_ERROR("Failed to load shader: '{0}'", file_path);
            return nullptr;
        }

        shaders[file_path] = create_ref<Shader>(shader_configuration);
        return shaders[file_path];
    }

    Shader::Shader(const ShaderConfiguration& shader_configuration) : configuration(shader_configuration)
    {
        auto& context = get_context();
        const u32 frame_count = context.get_frame_count();

        // Find total number of descriptor set layouts
        {
            u32 binding_count = 0;
            for (const auto& module : shader_configuration.shader_modules)
            {
                const auto& reflection = module.spv_module;
                binding_count = max(binding_count, reflection->descriptor_binding_count);
            }

            descriptor_set_layouts.resize(binding_count);
        }

        // Initialize all uniforms
        for (const auto& module : shader_configuration.shader_modules)
        {
            const auto& reflection = module.spv_module;

            const vk::ShaderStageFlags stage = static_cast<vk::ShaderStageFlags>(reflection->shader_stage);

            // Add vertex attributes sorted by location
            if (stage == vk::ShaderStageFlagBits::eVertex)
            {
                std::map<u32, SpvReflectInterfaceVariable*> sorted_input_variables;
                for (u32 i = 0; i < reflection->input_variable_count; i++)
                {
                    const auto& variable = reflection->input_variables[i];

                    // Filter built-in variables
                    if (variable->location < MAX_U32)
                    {
                        sorted_input_variables[variable->location] = variable;
                    }
                }

                u32 offset = 0;
                for (auto& input_variable_p : sorted_input_variables)
                {
                    auto& variable = input_variable_p.second;
                    const vk::Format format = static_cast<vk::Format>(variable->format);
                    u32 size = variable->numeric.scalar.width / 8;
                    size *= variable->numeric.vector.component_count > 0 ? variable->numeric.vector.component_count : 1;

                    add_attribute(format, size, offset);

                    offset += size;
                }
            }

            // Initialize push constants
            for (u32 i = 0; i < reflection->push_constant_block_count; i++)
            {
                auto* push_constant_block = new SpvReflectBlockVariable(reflection->push_constant_blocks[i]);

                // Already initialized
                if (uniforms_map.contains(push_constant_block->name)) continue;

                const str scope = push_constant_block->name;
                const u32 size = push_constant_block->size;

                uniforms_map[scope].push_constant_block = push_constant_block;

                // Store a pointer to each block member
                for (u32 m = 0; m < push_constant_block->member_count; m++)
                {
                    const auto& member = &push_constant_block->members[m];
                    uniforms_map[scope].members_cache[member->name] = member;
                }

                // @TODO: hardcoded stage
                push_constant_ranges.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment, 0, size));
            }

            for (u32 i = 0; i < reflection->descriptor_binding_count; i++)
            {
                auto& descriptor_binding = reflection->descriptor_bindings[i];

                // Already initialized
                if (uniforms_map.contains(descriptor_binding.name)) continue;

                const str scope = descriptor_binding.name;
                const u32 size = descriptor_binding.block.size;
                const vk::DescriptorType type = static_cast<vk::DescriptorType>(descriptor_binding.descriptor_type);

                uniforms_map[scope].descriptor_set_layouts.resize(frame_count);
                uniforms_map[scope].descriptor_sets.resize(frame_count);
                uniforms_map[scope].descriptor_binding = new SpvReflectDescriptorBinding(descriptor_binding);

                // Store a pointer to each block member
                for (u32 m = 0; m < descriptor_binding.block.member_count; m++)
                {
                    const auto& member = &uniforms_map[scope].descriptor_binding->block.members[m];
                    uniforms_map[scope].members_cache[member->name] = member;
                }

                // Create buffer for ubos
                if (type == vk::DescriptorType::eUniformBuffer)
                {
                    uniforms_map[scope].buffers.resize(frame_count);

                    for (u32 f = 0; f < frame_count; f++)
                    {
                        uniforms_map[scope].buffers[f].initialize(
                            size, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

                        auto& descriptor_set = uniforms_map[scope].descriptor_sets[f];
                        auto& descriptor_set_layout = uniforms_map[scope].descriptor_set_layouts[f];
                        auto& buffer = uniforms_map[scope].buffers[f];

                        DescriptorBuilder::create_descriptor_for_buffer(
                            descriptor_binding.binding, descriptor_set, descriptor_set_layout,
                            vk::DescriptorType::eUniformBuffer, buffer, size, 0);
                    }
                }

                // Create buffer for ssbos
                // @TODO: hardcoded size
                else if (type == vk::DescriptorType::eStorageBuffer)
                {
                    uniforms_map[scope].buffers.resize(frame_count);

                    const u64 BUFFER_SIZE = sizeof(mat4) * 10'000;
                    for (u32 f = 0; f < frame_count; f++)
                    {
                        uniforms_map[scope].buffers[f].initialize(
                            BUFFER_SIZE, vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

                        auto& descriptor_set = uniforms_map[scope].descriptor_sets[f];
                        auto& descriptor_set_layout = uniforms_map[scope].descriptor_set_layouts[f];
                        auto& buffer = uniforms_map[scope].buffers[f];

                        DescriptorBuilder::create_descriptor_for_buffer(
                            descriptor_binding.binding, descriptor_set, descriptor_set_layout,
                            vk::DescriptorType::eStorageBuffer, buffer, BUFFER_SIZE, 0);
                    }
                }

                // Create samplers for textures
                else if (type == vk::DescriptorType::eCombinedImageSampler)
                {
                    auto& app = get_application();
                    auto& renderer = app.get_renderer();
                    auto& texture_manager = app.get_texture_manager();

                    const auto& default_texture = texture_manager.get_default();

                    for (u32 f = 0; f < frame_count; f++)
                    {
                        auto& descriptor_set = uniforms_map[scope].descriptor_sets[f];
                        auto& descriptor_set_layout = uniforms_map[scope].descriptor_set_layouts[f];

                        std::vector<ref<RendererImage>> textures;
                        for (u32 sampler_idx = 0; sampler_idx < descriptor_binding.count; sampler_idx++)
                        {
                            textures.push_back(renderer.get_renderer_image(default_texture.get()));
                        }

                        DescriptorBuilder::create_descriptor_for_textures(descriptor_binding.binding, textures,
                                                                          descriptor_set, descriptor_set_layout);
                    }
                }

                // Descriptor type not handled
                else
                {
                    ASSERT(false, "Descriptor type " + vk::to_string(type) + " not supported");
                }

                descriptor_set_layouts[descriptor_binding.set] = uniforms_map[scope].descriptor_set_layouts[0];
            }
        }

        pipeline = create_unique<Pipeline>(*this);
    }

    Shader::~Shader()
    {
        auto& context = get_context();
        context.get_device().waitIdle();

        for (auto& uniform_p : uniforms_map)
        {
            auto& ubo = uniform_p.second;
            for (auto& buffer : ubo.buffers)
            {
                buffer.shutdown();
            }

            delete ubo.descriptor_binding;
            delete ubo.push_constant_block;

            ubo.descriptor_binding = nullptr;
            ubo.push_constant_block = nullptr;
        }

        for (auto& shader_module : configuration.shader_modules)
        {
            context.get_device().destroyShaderModule(*shader_module.module);
            spvReflectDestroyShaderModule(const_cast<SpvReflectShaderModule*>(shader_module.spv_module));

            delete shader_module.module;
            delete shader_module.spv_module;

            shader_module.module = nullptr;
            shader_module.spv_module = nullptr;
        }
    }

    void Shader::bind() { pipeline->bind(); }

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

    void Shader::set_uniform(const str& scope, const str& name, const void* data, const u64 data_offset)
    {
        auto uniform_it = uniforms_map.find(scope);
        if (uniform_it == uniforms_map.end())
        {
            // Uniform not found
            LOG_ERROR("Uniform scope '{0}' not found", scope);
            return;
        }

        auto& context = get_context();
        const u32 curr_frame_number = context.get_curr_frame_number();
        const auto& cmd = context.get_curr_frame().command_buffer;

        auto& ubo = uniform_it->second;
        auto& members_cache = ubo.members_cache;

        auto it = members_cache.find(name);
        if (it != members_cache.end())
        {
            const u64 offset = it->second->offset;
            const u64 size = it->second->size;

            // Check if uniform is a push constant or ubo
            if (ubo.push_constant_block != nullptr)
            {
                vk::PipelineLayout pipeline_layout =
                    *reinterpret_cast<const vk::PipelineLayout*>(pipeline->get_layout());

                // @TODO: hardcoded shader stage
                cmd.get_handle().pushConstants(pipeline_layout, vk::ShaderStageFlagBits::eFragment,
                                               offset + data_offset, size, data);
            }

            else
            {
                auto& buffer = ubo.buffers[curr_frame_number];
                buffer.copy(data, size, offset + data_offset);

                bind_descriptor(ubo.descriptor_binding->set, ubo.descriptor_sets[curr_frame_number]);
            }

            return;
        }

        // Uniform not found
        LOG_ERROR("Uniform '{0}' not found in scope '{1}'", name, scope);
    }

    void Shader::set_texture(const str& name, Image* texture)
    {
        auto it = uniforms_map.find(name);
        if (it == uniforms_map.end())
        {
            // Uniform not found
            LOG_ERROR("Uniform texture '{0}' not found", name);
            return;
        }

        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& context = get_context();

        const u32 curr_frame_number = context.get_curr_frame_number();

        auto& ubo = it->second;

        // Create descriptor for this texture
        if (texture_descriptor_sets.count(texture) == 0)
        {
            vk::DescriptorSet descriptor_set;
            vk::DescriptorSetLayout descriptor_set_layout;

            auto renderer_texture = renderer.get_renderer_image(texture);

            DescriptorBuilder::create_descriptor_for_textures(ubo.descriptor_binding->binding, {renderer_texture},
                                                              descriptor_set, descriptor_set_layout);

            texture_descriptor_sets[texture] = descriptor_set;
        }

        ubo.descriptor_sets[curr_frame_number] = texture_descriptor_sets[texture];

        bind_descriptor(ubo.descriptor_binding->set, ubo.descriptor_sets[curr_frame_number]);
    }

    void Shader::set_texture(const str& name, RendererImage* texture)
    {
        auto it = uniforms_map.find(name);
        if (it == uniforms_map.end())
        {
            // Uniform not found
            LOG_ERROR("Uniform texture '{0}' not found", name);
            return;
        }

        auto& context = get_context();

        const u32 curr_frame_number = context.get_curr_frame_number();

        auto& ubo = it->second;

        // Create descriptor for this texture
        if (texture_descriptor_sets.count(static_cast<void*>(texture)) == 0)
        {
            vk::DescriptorSet descriptor_set;
            vk::DescriptorSetLayout descriptor_set_layout;

            DescriptorBuilder::create_descriptor_for_textures(ubo.descriptor_binding->binding, {texture},
                                                              descriptor_set, descriptor_set_layout);

            texture_descriptor_sets[static_cast<void*>(texture)] = descriptor_set;
        }

        ubo.descriptor_sets[curr_frame_number] = texture_descriptor_sets[static_cast<void*>(texture)];

        bind_descriptor(ubo.descriptor_binding->set, ubo.descriptor_sets[curr_frame_number]);
    }

    void Shader::set_material(const str& name, Material* material)
    {
        auto it = uniforms_map.find(name);
        if (it == uniforms_map.end())
        {
            // Uniform not found
            LOG_ERROR("Uniform material '{0}' not found", name);
            return;
        }

        auto& app = get_application();
        auto& renderer = app.get_renderer();
        auto& texture_manager = app.get_texture_manager();
        auto& context = get_context();

        const u32 curr_frame_number = context.get_curr_frame_number();

        auto& ubo = it->second;

        // @TODO: this blocks the main thread and should be paralelized when the renderer supports it.
        // Create/Update descriptor for this material
        if (texture_descriptor_sets.count(material) == 0 ||
            material->loading_state == MaterialLoadingState::LoadingFinished)
        {
            vk::DescriptorSet descriptor_set;
            vk::DescriptorSetLayout descriptor_set_layout;

            std::vector<ref<RendererImage>> renderer_textures;
            for (const auto& texture_p : material->textures)
            {
                const auto& texture_name = texture_p.second;
                const auto& texture = texture_manager.get(texture_name);
                const auto& renderer_texture = renderer.get_renderer_image(texture.get());

                renderer_textures.push_back(renderer_texture);
            }

            DescriptorBuilder::create_descriptor_for_textures(ubo.descriptor_binding->binding, renderer_textures,
                                                              descriptor_set, descriptor_set_layout);

            texture_descriptor_sets[material] = descriptor_set;

            material->loading_state = MaterialLoadingState::UploadedToGPU;
        }

        ubo.descriptor_sets[curr_frame_number] = texture_descriptor_sets[material];

        bind_descriptor(ubo.descriptor_binding->set, ubo.descriptor_sets[curr_frame_number]);
    }

    void Shader::bind_descriptor(const u32 set, const vk::DescriptorSet& descriptor_set)
    {
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;

        command_buffer.bind_descriptor_set(vk::PipelineBindPoint::eGraphics,
                                           *static_cast<const vk::PipelineLayout*>(pipeline->get_layout()), set,
                                           descriptor_set);
    }

    const ShaderConfiguration& Shader::get_shader_configuration() const { return configuration; }

    const std::vector<vk::VertexInputBindingDescription>& Shader::get_vertex_bindings() const
    {
        return vertex_bindings;
    }

    const std::vector<vk::VertexInputAttributeDescription>& Shader::get_vertex_attributes() const
    {
        return vertex_attributes;
    }

    const std::vector<vk::DescriptorSetLayout>& Shader::get_descriptor_set_layouts() const
    {
        return descriptor_set_layouts;
    }

    const std::vector<vk::PushConstantRange>& Shader::get_push_constant_ranges() const { return push_constant_ranges; }
};  // namespace mag
