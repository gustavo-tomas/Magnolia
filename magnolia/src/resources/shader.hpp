#pragma once

#include <map>
#include <vector>

#include "core/types.hpp"
#include "resources/resource.hpp"

namespace mag
{
    enum class ShaderResourceStage
    {
        Vertex,
        Fragment
    };

    struct ShaderResource : public IResource
    {
            str name = "";
            str glsl_file_path = "";

            std::map<ShaderResourceStage, std::vector<u8>> stages;
    };

    namespace resource
    {
        ref<ShaderResource> get_shader(const str& name);
    };  // namespace resource
};      // namespace mag
