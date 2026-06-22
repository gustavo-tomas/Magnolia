#pragma once

#include <vector>

#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"
#include "magnolia/resources/resource.hpp"

namespace mag
{
    struct Vertex
    {
            math::vec3 position;
            math::vec3 normal;
            math::vec2 tex_coords;
            math::vec3 tangent;
            math::vec3 bitangent;
    };

    struct Mesh
    {
            u32 base_vertex = 0;
            u32 base_index = 0;
            u32 index_count = 0;
            u32 material_index = 0;
            math::vec3 aabb_min;
            math::vec3 aabb_max;
    };

    struct ModelResource : public IResource
    {
            std::vector<Mesh> meshes;
            std::vector<Vertex> vertices;
            std::vector<u32> indices;
            std::vector<ref<MaterialResource>> materials;
    };

    namespace resource
    {
        b8 load_sync(const str& file_path, ResourceManager* rm, ModelResource* resource);
    };  // namespace resource
};  // namespace mag

namespace mag::fs
{
    void to_binary(std::ostringstream& ss, const Vertex& data);
    void from_binary(std::istringstream& ss, Vertex& data);

    void to_binary(std::ostringstream& ss, const Mesh& data);
    void from_binary(std::istringstream& ss, Mesh& data);

    void to_binary(std::ostringstream& ss, const ModelResource& data);
    void from_binary(std::istringstream& ss, ModelResource& data);
};  // namespace mag::fs
