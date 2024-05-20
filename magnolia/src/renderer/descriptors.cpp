#include "renderer/descriptors.hpp"

#include <algorithm>

#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "renderer/image.hpp"
#include "renderer/model.hpp"
#include "renderer/shader.hpp"

namespace mag
{
    // DescriptorLayoutCache
    // -----------------------------------------------------------------------------------------------------------------
    void DescriptorLayoutCache::initialize() {}

    void DescriptorLayoutCache::shutdown()
    {
        auto& context = get_context();

        // delete every descriptor layout held
        for (const auto& layout_pair : layout_cache)
        {
            const auto& layout = layout_pair.second;
            context.get_device().destroyDescriptorSetLayout(layout);
        }
    }

    vk::DescriptorSetLayout DescriptorLayoutCache::create_descriptor_layout(
        const vk::DescriptorSetLayoutCreateInfo* info)
    {
        auto& context = get_context();

        DescriptorLayoutInfo layout_info = {};
        layout_info.bindings.reserve(info->bindingCount);
        b8 is_sorted = true;
        i32 last_binding = -1;

        // copy from the direct info struct into our own one
        for (u32 i = 0; i < info->bindingCount; i++)
        {
            layout_info.bindings.push_back(info->pBindings[i]);

            // check that the bindings are in strict increasing order
            if (static_cast<i32>(info->pBindings[i].binding) > last_binding)
                last_binding = info->pBindings[i].binding;

            else
                is_sorted = false;
        }

        // sort the bindings if they aren't in order
        if (!is_sorted)
        {
            std::sort(layout_info.bindings.begin(), layout_info.bindings.end(),
                      [](const vk::DescriptorSetLayoutBinding& a, const vk::DescriptorSetLayoutBinding& b)
                      { return a.binding < b.binding; });
        }

        // try to grab from cache
        auto it = layout_cache.find(layout_info);
        if (it != layout_cache.end())
            return it->second;

        else
        {
            // create a new one (not found)
            vk::DescriptorSetLayout layout;
            VK_CHECK(context.get_device().createDescriptorSetLayout(info, nullptr, &layout));

            // add to cache
            layout_cache[layout_info] = layout;
            return layout;
        }
    }

    b8 DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
    {
        if (other.bindings.size() != bindings.size()) return false;

        // compare each of the bindings is the same. Bindings are sorted so they will match
        else
        {
            for (u64 i = 0; i < bindings.size(); i++)
            {
                if (other.bindings[i].binding != bindings[i].binding) return false;
                if (other.bindings[i].descriptorType != bindings[i].descriptorType) return false;
                if (other.bindings[i].descriptorCount != bindings[i].descriptorCount) return false;
                if (other.bindings[i].stageFlags != bindings[i].stageFlags) return false;
            }

            return true;
        }
    }

    size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
    {
        using std::hash;
        using std::size_t;

        // !TODO: double check this algorithm
        size_t result = hash<size_t>()(bindings.size());
        for (const vk::DescriptorSetLayoutBinding& b : bindings)
        {
            // pack the binding data into a single int64. Not fully correct but it's ok
            size_t binding_hash = b.binding | static_cast<u32>(b.descriptorType) << 8 | b.descriptorCount << 16 |
                                  static_cast<u32>(b.stageFlags) << 24;

            // shuffle the packed binding data and xor it with the main hash
            result ^= hash<size_t>()(binding_hash);
        }

        return result;
    }

    size_t DescriptorLayoutCache::DescriptorLayoutHash::operator()(const DescriptorLayoutInfo& k) const
    {
        return k.hash();
    }

    // DescriptorBuilder
    // -----------------------------------------------------------------------------------------------------------------
    Descriptor DescriptorBuilder::build_layout(const SpvReflectShaderModule& shader_reflection, const u32 set)
    {
        ASSERT(shader_reflection.descriptor_set_count > set, "Invalid descriptor set");

        auto& context = get_context();
        auto& device = context.get_device();
        auto& cache = context.get_descriptor_layout_cache();

        Descriptor descriptor;

        // @TODO: setting both vertex and fragment stages
        // const vk::ShaderStageFlagBits stage = static_cast<vk::ShaderStageFlagBits>(shader_reflection.shader_stage);
        const vk::ShaderStageFlags stage = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

        // Create the descriptor binding for the layout
        const SpvReflectDescriptorSet& descriptor_set = shader_reflection.descriptor_sets[set];

        // Create all bindings for the specified set
        std::vector<vk::DescriptorSetLayoutBinding> bindings;
        for (u32 b = 0; b < descriptor_set.binding_count; b++)
        {
            const u32 binding = descriptor_set.bindings[b]->binding;
            const u32 count = descriptor_set.bindings[b]->count;
            const vk::DescriptorType type =
                static_cast<vk::DescriptorType>(descriptor_set.bindings[b]->descriptor_type);

            vk::DescriptorSetLayoutBinding new_binding(binding, type, count, stage);
            bindings.push_back(new_binding);
        }

        // Build layout
        const vk::DescriptorSetLayoutCreateInfo layout_info(vk::DescriptorSetLayoutCreateFlagBits::eDescriptorBufferEXT,
                                                            bindings);
        descriptor.layout = cache.create_descriptor_layout(&layout_info);

        auto descriptor_buffer_properties = context.get_descriptor_buffer_properties();

        // 2. Get size
        // Get set layout descriptor sizes and adjust them to satisfy alignment requirements.
        const u64 alignment = descriptor_buffer_properties.descriptorBufferOffsetAlignment;
        descriptor.size = device.getDescriptorSetLayoutSizeEXT(descriptor.layout);
        descriptor.size = (descriptor.size + alignment - 1) & ~(alignment - 1);

        // Get descriptor bindings offsets as descriptors are placed inside set layout by those offsets.
        // @TODO: The binding can be non zero depending on the driver:
        // https://github.com/KhronosGroup/Vulkan-Samples/tree/main/samples/extensions/descriptor_buffer_basic
        descriptor.offset = device.getDescriptorSetLayoutBindingOffsetEXT(descriptor.layout, 0);

        return descriptor;
    }

    void DescriptorBuilder::build(const Descriptor& descriptor, const std::vector<Buffer>& data_buffers)
    {
        auto& context = get_context();
        auto& device = context.get_device();
        auto descriptor_buffer_properties = context.get_descriptor_buffer_properties();

        // 4. Put the descriptors into buffers
        char* descriptor_buffer_data = static_cast<char*>(descriptor.buffer.get_data());

        for (u64 i = 0; i < data_buffers.size(); i++)
        {
            const vk::DescriptorAddressInfoEXT address_info(data_buffers[i].get_device_address(),
                                                            data_buffers[i].get_size(), vk::Format::eUndefined);

            const vk::DescriptorGetInfoEXT descriptor_info(vk::DescriptorType::eUniformBuffer, {&address_info});

            const u64 offset = i ? i * descriptor.size + descriptor.offset : 0;

            device.getDescriptorEXT(descriptor_info, descriptor_buffer_properties.uniformBufferDescriptorSize,
                                    descriptor_buffer_data + offset);
        }
    }

    void DescriptorBuilder::build(const Descriptor& descriptor, const std::vector<std::shared_ptr<Image>>& images)
    {
        auto& context = get_context();
        auto& device = context.get_device();
        auto descriptor_buffer_properties = context.get_descriptor_buffer_properties();

        // 4. Put the descriptors into buffers
        char* descriptor_buffer_data = static_cast<char*>(descriptor.buffer.get_data());

        for (u64 i = 0; i < images.size(); i++)
        {
            const vk::DescriptorImageInfo image_info(images[i]->get_sampler().get_handle(),
                                                     images[i]->get_image_view());

            const vk::DescriptorGetInfoEXT descriptor_info(vk::DescriptorType::eCombinedImageSampler, {&image_info});

            const u64 offset = i * descriptor.size + descriptor.offset;

            device.getDescriptorEXT(descriptor_info, descriptor_buffer_properties.combinedImageSamplerDescriptorSize,
                                    descriptor_buffer_data + offset);
        }
    }

    // Descriptor Buffer Limits
    // By limiting the number of models/textures we can initialize the descriptor buffers only once and simply rebuild
    // get the descriptor sets again when we modify the uniform/texture data. @TODO: we may want to extend this approach
    // and first check if the descriptor buffer supports the size and also make the limits configurable.
    const u32 MAX_NUMBER_OF_GLOBALS = 1;      // Only one global buffer
    const u32 MAX_NUMBER_OF_INSTANCES = 999;  // The rest is instance buffers
    const u32 MAX_NUMBER_OF_UNIFORMS = MAX_NUMBER_OF_GLOBALS + MAX_NUMBER_OF_INSTANCES;
    const u32 MAX_NUMBER_OF_TEXTURES = 1000;

    // DescriptorCache
    // -----------------------------------------------------------------------------------------------------------------
    void DescriptorCache::build_descriptors_from_shader(const Shader& shader)
    {
        const u32 frame_count = get_context().get_frame_count();

        uniform_descriptors.resize(frame_count);
        image_descriptors.resize(frame_count);
        data_buffers.resize(frame_count);

        auto vertex_module = shader.get_modules()[0];
        auto fragment_module = shader.get_modules()[1];
        auto uniforms = shader.get_uniforms();

        // Create descriptors
        for (u64 i = 0; i < frame_count; i++)
        {
            // @TODO: hardcoded values
            if (vertex_module->get_reflection().descriptor_set_count > 0)
            {
                uniform_inited = true;

                uniform_descriptors[i] = DescriptorBuilder::build_layout(vertex_module->get_reflection(), 0);

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

            if (fragment_module->get_reflection().descriptor_set_count > 2)
            {
                image_inited = true;

                image_descriptors[i] = DescriptorBuilder::build_layout(fragment_module->get_reflection(), 2);

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
                const u64 buffer_size =
                    b < MAX_NUMBER_OF_GLOBALS ? uniforms["u_global"].data.size() : uniforms["u_instance"].data.size();

                Buffer buffer;
                buffer.initialize(buffer_size,
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                  VMA_MEMORY_USAGE_AUTO, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

                data_buffers[i].push_back(buffer);
            }

            DescriptorBuilder::build(uniform_descriptors[i], data_buffers[i]);
        }
    }

    DescriptorCache::~DescriptorCache()
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

    void DescriptorCache::bind()
    {
        auto& context = get_context();
        auto& command_buffer = context.get_curr_frame().command_buffer;
        const u32 curr_frame_number = context.get_curr_frame_number();

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

    void DescriptorCache::add_image_descriptors_for_model(const Model& model)
    {
        auto& context = get_context();

        const u32 frame_count = context.get_frame_count();

        // Put all models textures in a single array
        for (auto& mesh : model.meshes)
        {
            if (auto texture = mesh.material->diffuse_texture)
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

    void DescriptorCache::set_descriptor_buffer_offset(const vk::PipelineLayout& pipeline_layout, const u32 first_set,
                                                       const u32 buffer_indices, const u64 buffer_offsets)
    {
        auto& command_buffer = get_context().get_curr_frame().command_buffer;

        const auto pipeline_bind_point = vk::PipelineBindPoint::eGraphics;

        command_buffer.get_handle().setDescriptorBufferOffsetsEXT(pipeline_bind_point, pipeline_layout, first_set,
                                                                  buffer_indices, buffer_offsets);
    }

    void DescriptorCache::set_offset_global(const vk::PipelineLayout& pipeline_layout)
    {
        set_descriptor_buffer_offset(pipeline_layout, 0, 0, 0);
    }

    void DescriptorCache::set_offset_instance(const vk::PipelineLayout& pipeline_layout, const u32 instance)
    {
        const u32 curr_frame_number = get_context().get_curr_frame_number();
        const u64 buffer_offsets = (instance + 1) * uniform_descriptors[curr_frame_number].size;

        set_descriptor_buffer_offset(pipeline_layout, 1, 0, buffer_offsets);
    }

    void DescriptorCache::set_offset_material(const vk::PipelineLayout& pipeline_layout, const u32 index)
    {
        const u32 curr_frame_number = get_context().get_curr_frame_number();
        const u64 buffer_offsets = index * image_descriptors[curr_frame_number].size;

        set_descriptor_buffer_offset(pipeline_layout, 2, 1, buffer_offsets);
    }
};  // namespace mag
