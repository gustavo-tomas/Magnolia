#pragma once

#include <vector>

#include "core/types.hpp"
#include "math/types.hpp"
#include "resources/resource.hpp"

namespace mag
{
    using namespace mag::math;

    struct Vertex
    {
            vec3 position;
            vec3 normal;
            vec2 tex_coords;
            vec3 tangent;
            vec3 bitangent;
    };

    struct Mesh
    {
            u32 base_vertex;
            u32 base_index;
            u32 index_count;
            u32 material_index;
            vec3 aabb_min;
            vec3 aabb_max;
    };

    struct Model : public IResource
    {
            str name = "";
            str file_path = "";

            std::vector<Mesh> meshes;
            std::vector<Vertex> vertices;
            std::vector<u32> indices;
            std::vector<str> materials;
    };

    namespace resource
    {
        MAG_API ref<Model> get_model(const str& name);
    };  // namespace resource
};      // namespace mag
