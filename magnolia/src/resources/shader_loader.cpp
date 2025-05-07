// this header on top
#include "resources/resource_loader.hpp"
// this header on top

#include "core/buffer.hpp"
#include "core/logger.hpp"
#include "platform/file_system.hpp"
#include "resources/shader.hpp"

namespace mag
{
    namespace resource
    {
        struct ShaderStageData
        {
                str extension;
                ShaderStage stage;
        };

        static const std::map<str, ShaderStageData> shader_stage_map = {
            {"Vertex", {.extension = ".vert", .stage = ShaderStage::Vertex}},
            {"Fragment", {.extension = ".frag", .stage = ShaderStage::Fragment}}};

        b8 load(const str& file_path, Shader* shader)
        {
            fs::json data;

            if (!fs::read_json_data(file_path, data))
            {
                LOG_ERROR("Failed to load shader file: '{0}'", file_path);
                return false;
            }

            if (!data.contains("Name") || !data.contains("Stages") || !data.contains("File"))
            {
                LOG_ERROR("Shader file '{0}' has incomplete fields", file_path);
                return false;
            }

            const str name = data["Name"];
            const str glsl_file_path = data["File"];
            const std::vector<str> stages = data["Stages"].get<std::vector<str>>();

            for (const str& stage : stages)
            {
                // Build the binary name from the glsl name
                const str binary_file_path =
                    MAG_BUILD_SHADER_NAME(fs::get_file_name(glsl_file_path) + shader_stage_map.at(stage).extension);

                Buffer buffer;
                const b8 result = fs::read_binary_data(binary_file_path, buffer);

                if (!result)
                {
                    LOG_ERROR("Failed to load native model binary file: '{0}'", binary_file_path);
                    return false;
                }

                shader->stages[shader_stage_map.at(stage).stage] = buffer.data;
            }

            shader->name = name;
            shader->glsl_file_path = glsl_file_path;

            LOG_SUCCESS("Loaded shader: {0}", file_path);
            return true;
        }
    };  // namespace resource
};      // namespace mag
