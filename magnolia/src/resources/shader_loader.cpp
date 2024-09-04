#include "resources/shader_loader.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"

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
        const json files = data["Files"];

        // Vertex and fragment shaders are necessary, the other stages are optional
        if (!files.contains("Vertex") || !files.contains("Fragment"))
        {
            LOG_ERROR("Shader '{0}' is missing vertex/fragment shaders");
            return false;
        }

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

        const str vertex_file_path = shader_folder + str(files["Vertex"]);
        const str fragment_file_path = shader_folder + str(files["Fragment"]);

        ShaderModule vertex_module;
        ShaderModule fragment_module;

        if (!load_module(vertex_file_path, &vertex_module))
        {
            LOG_ERROR("Failed to load module: '{0}'", vertex_file_path);
            return false;
        }

        if (!load_module(fragment_file_path, &fragment_module))
        {
            LOG_ERROR("Failed to load module: '{0}'", fragment_file_path);
            return false;
        }

        const json pipeline_data = data["Pipeline"];

        shader->name = shader_name;
        shader->file_path = file_path;

        shader->shader_modules.push_back(vertex_module);
        shader->shader_modules.push_back(fragment_module);

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
