#pragma once

#include <map>
#include <memory>
#include <vector>

#include "core/types.hpp"

namespace mag
{
    using namespace math;

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
    };

    struct Model
    {
            str name = "";
            str file_path = "";

            std::vector<Mesh> meshes;
            std::vector<Vertex> vertices;
            std::vector<u32> indices;
            std::vector<str> materials;
    };

    class ModelManager
    {
        public:
            std::shared_ptr<Model> get(const str& name);

        private:
            std::map<str, std::shared_ptr<Model>> models;
    };
};  // namespace mag