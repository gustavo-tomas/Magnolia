// this header on top
#include "resources/resource_loader.hpp"
// this header on top

#include <vulkan/vulkan.hpp>

#include "core/buffer.hpp"
#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "renderer/context.hpp"
#include "renderer/shader.hpp"
#include "spirv_reflect.h"

// @TODO: for now i think the shader manager should be in charge of compiling the shaders. its ugly and slow,
// but it will allow for greater flexibility and will also decouple the shader compilation from the editor (rn
// we can only build shaders for the editor and nothing else). we should also be careful with collisions so that
// we dont overwrite any shader. in the end, we should be able to separate editor assets from the application
// assets without worrying too much about build directories.

namespace mag
{
    namespace resource
    {
        static b8 compile_module(const str& input_file_path, const str& output_file_path,
                                 const std::vector<str>& include_paths);
        static b8 load_module(const str& file_path, ShaderModule* shader_module);

        b8 load(const str& file_path, ShaderConfiguration* shader, const b8 force_recompilation)
        {
            if (!shader)
            {
                LOG_ERROR("Invalid shader ptr");
                return false;
            }

            fs::json data;

            if (!fs::read_json_data(file_path, data))
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

            b8 contains_vertex_stage = false;
            b8 contains_fragment_stage = false;

            std::vector<str> shader_modules = data["Files"];

            // Compile shader modules
            for (auto& shader_module_file : shader_modules)
            {
                // Skip shader compilation if binary module already exists
                const str bin_shader_file_path = MAG_BUILD_SHADER_NAME(shader_module_file);

                if (!force_recompilation && fs::exists(bin_shader_file_path))
                {
                    continue;
                }

                const str shader_file_path = fs::path(file_path).parent_path().string() / fs::path(shader_module_file);
                const std::vector<str> include_paths = {"sprout_editor/assets/shaders"};

                if (!compile_module(shader_file_path, bin_shader_file_path, include_paths))
                {
                    LOG_ERROR("Failed to compile module: '{0}'", shader_file_path);
                    return false;
                }
            }

            // Load compiled shader modules
            for (auto& shader_module_file : shader_modules)
            {
                const str bin_shader_file_path = MAG_BUILD_SHADER_NAME(shader_module_file);

                ShaderModule shader_module;

                if (!load_module(bin_shader_file_path, &shader_module))
                {
                    LOG_ERROR("Failed to load module: '{0}'", bin_shader_file_path);
                    return false;
                }

                if (shader_module.spv_module->shader_stage ==
                    static_cast<SpvReflectShaderStageFlagBits>(vk::ShaderStageFlagBits::eVertex))
                {
                    contains_vertex_stage = true;
                }

                if (shader_module.spv_module->shader_stage ==
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

            const fs::json pipeline_data = data["Pipeline"];

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

            shader->color_write_enabled = pipeline_data["ColorWrite"]["Enabled"].get<b8>();
            shader->depth_write_enabled = pipeline_data["DepthWrite"]["Enabled"].get<b8>();

            return true;
        }

        static b8 compile_module(const str& input_file_path, const str& output_file_path,
                                 const std::vector<str>& include_paths)
        {
            LOG_INFO("Compiling shader module '{0}'...", input_file_path);

            // Create directories if they dont exist
            fs::create_directories(fs::path(output_file_path).parent_path());

            str compile_shader_cmd = MAG_EXT_GLSLC;
            for (const str& include_path : include_paths)
            {
                compile_shader_cmd += " -I" + include_path;
            }
            compile_shader_cmd += " " + input_file_path + " -o " + output_file_path;

            // Execute glslc
            return system(compile_shader_cmd.c_str()) == 0;
        }

        static b8 load_module(const str& file_path, ShaderModule* shader_module)
        {
            auto& context = get_context();

            Buffer buffer;
            if (!fs::read_binary_data(file_path, buffer))
            {
                LOG_ERROR("Failed to load module: '{0}'", file_path);
                return false;
            }

            vk::ShaderModule* module = new vk::ShaderModule();

            *module = context.get_device().createShaderModule(
                vk::ShaderModuleCreateInfo({}, buffer.get_size(), buffer.cast<u32>()));

            // Generate reflection data for a shader
            SpvReflectShaderModule* spv_module = new SpvReflectShaderModule();
            SpvReflectResult result = spvReflectCreateShaderModule(buffer.get_size(), buffer.cast<u32>(), spv_module);
            VK_CHECK(VK_CAST(result));

            shader_module->module = module;
            shader_module->spv_module = spv_module;

            return true;
        }
    };  // namespace resource
};      // namespace mag
