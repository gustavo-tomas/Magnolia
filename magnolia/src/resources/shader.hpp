#pragma once

#include <map>
#include <vector>

#include "core/types.hpp"
#include "resources/resource.hpp"

namespace mag
{
    enum class ShaderStage
    {
        Vertex,
        Fragment
    };

    struct Shader : public IResource
    {
            str name = "";
            str glsl_file_path = "";

            std::map<ShaderStage, std::vector<u8>> stages;
    };

    namespace resource
    {
        ref<Shader> get_shader(const str& name);
    };  // namespace resource
};      // namespace mag
