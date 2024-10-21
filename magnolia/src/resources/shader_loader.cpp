#include "resources/shader_loader.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "renderer/context.hpp"
#include "renderer/shader.hpp"

namespace mag
{
    b8 ShaderLoader::load(const str& file_path, ShaderConfiguration* shader)
    {
        if (!shader)
        {
            LOG_ERROR("Invalid shader ptr");
            return false;
        }

        auto& app = get_application();
        auto& file_system = app.get_file_system();

        json data;

        if (!file_system.read_json_data(file_path, data))
        {
            LOG_ERROR("Failed to load shader file: {0}", file_path);
            return false;
        }

        if (!data.contains("Files"))
        {
            LOG_ERROR("Shader '{0}' has no shader stages", file_path);
            return false;
        }

        if (!data.contains("Pipeline"))
        {
            LOG_ERROR("Shader '{0}' has no pipeline configuration", file_path);
            return false;
        }

        const str shader_name = data["Shader"];

        // @TODO: cleanup
        str shader_folder = "shaders/";
        {
            // Shaders
            const std::filesystem::path cwd = std::filesystem::current_path();
            const str last_folder = cwd.filename().string();
            str system = "linux";

// @TODO: clean this up (maybe use a filesystem class)
#if defined(_WIN32)
            system = "windows";
#endif
            if (last_folder == "Magnolia") shader_folder = "build/" + system + "/" + shader_folder;
        }
        // @TODO: cleanup

        b8 contains_vertex_stage = false;
        b8 contains_fragment_stage = false;

        std::vector<str> shader_modules = data["Files"];
        for (auto& shader_module_file : shader_modules)
        {
            const str shader_module_path = shader_folder + shader_module_file;

            ShaderModule shader_module;

            if (!load_module(shader_module_path, &shader_module))
            {
                LOG_ERROR("Failed to load module: '{0}'", shader_module_path);
                return false;
            }

            if (shader_module.spv_module.shader_stage ==
                static_cast<SpvReflectShaderStageFlagBits>(vk::ShaderStageFlagBits::eVertex))
            {
                contains_vertex_stage = true;
            }

            if (shader_module.spv_module.shader_stage ==
                static_cast<SpvReflectShaderStageFlagBits>(vk::ShaderStageFlagBits::eFragment))
            {
                contains_fragment_stage = true;
            }

            shader->shader_modules.push_back(shader_module);
        }

        // Vertex and fragment shaders are necessary, the other stages are optional
        if (!contains_vertex_stage || !contains_fragment_stage)
        {
            LOG_ERROR("Shader '{0}' is missing vertex/fragment shaders", file_path);
            return false;
        }

        const json pipeline_data = data["Pipeline"];

        shader->name = shader_name;
        shader->file_path = file_path;

        shader->topology = pipeline_data["InputAssembly"]["Topology"];
        shader->polygon_mode = pipeline_data["Rasterization"]["PolygonMode"];
        shader->cull_mode = pipeline_data["Rasterization"]["CullMode"];

        shader->color_blend_enabled = pipeline_data["ColorBlend"]["Enabled"].get<b8>();

        if (shader->color_blend_enabled)
        {
            shader->color_blend_op = pipeline_data["ColorBlend"]["ColorBlendOp"];
            shader->alpha_blend_op = pipeline_data["ColorBlend"]["AlphaBlendOp"];
            shader->src_color_blend_factor = pipeline_data["ColorBlend"]["SrcColorBlendFactor"];
            shader->dst_color_blend_factor = pipeline_data["ColorBlend"]["DstColorBlendFactor"];
            shader->src_alpha_blend_factor = pipeline_data["ColorBlend"]["SrcAlphaBlendFactor"];
            shader->dst_alpha_blend_factor = pipeline_data["ColorBlend"]["DstAlphaBlendFactor"];
        }

        return true;
    }

    b8 ShaderLoader::load_module(const str& file_path, ShaderModule* shader_module)
    {
        auto& context = get_context();
        auto& app = get_application();
        auto& file_system = app.get_file_system();

        Buffer buffer;
        if (!file_system.read_binary_data(file_path, buffer))
        {
            LOG_ERROR("Failed to load module: '{0}'", file_path);
            return false;
        }

        const vk::ShaderModule module = context.get_device().createShaderModule(
            vk::ShaderModuleCreateInfo({}, buffer.get_size(), buffer.cast<u32>()));

        // Generate reflection data for a shader
        SpvReflectShaderModule spv_module = {};
        SpvReflectResult result = spvReflectCreateShaderModule(buffer.get_size(), buffer.cast<u32>(), &spv_module);
        VK_CHECK(VK_CAST(result));

        shader_module->module = module;
        shader_module->spv_module = spv_module;

        return true;
    }
};  // namespace mag
