#pragma once

#include <map>
#include <vector>

#include "core/types.hpp"
#include "math/types.hpp"
#include "math/vec.hpp"

namespace mag
{
#define DEFAULT_MODEL_NAME "__mag_default_model__"

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
            ModelManager();

            ref<Model> get(const str& name);
            ref<Model> get_default();

        private:
            std::map<str, ref<Model>> models;
    };
};  // namespace mag
