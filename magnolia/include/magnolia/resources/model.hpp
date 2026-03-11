#pragma once

#include <vector>

#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"
#include "magnolia/platform/serializer_fwd.hpp"
#include "magnolia/resources/resource.hpp"

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

    struct ModelResource : public IResource
    {
            std::vector<Mesh> meshes;
            std::vector<Vertex> vertices;
            std::vector<u32> indices;
            std::vector<ref<MaterialResource>> materials;
    };

    namespace resource
    {
        class ModelLoader : public IResourceLoader
        {
            public:
                ModelLoader(ResourceManager* resource_manager);
                ~ModelLoader() override;

                IResource* load_sync(const str& file_path) override;

            private:
                ResourceManager* resource_manager = nullptr;
        };
    };  // namespace resource
};  // namespace mag

namespace mag::fs
{
    inline void to_binary(std::ostringstream& ss, const Vertex& data)
    {
        to_binary(ss, data.position);
        to_binary(ss, data.normal);
        to_binary(ss, data.tex_coords);
        to_binary(ss, data.tangent);
        to_binary(ss, data.bitangent);
    }

    inline void from_binary(std::istringstream& ss, Vertex& data)
    {
        from_binary(ss, data.position);
        from_binary(ss, data.normal);
        from_binary(ss, data.tex_coords);
        from_binary(ss, data.tangent);
        from_binary(ss, data.bitangent);
    }

    inline void to_binary(std::ostringstream& ss, const Mesh& data)
    {
        to_binary(ss, data.base_vertex);
        to_binary(ss, data.base_index);
        to_binary(ss, data.index_count);
        to_binary(ss, data.material_index);
        to_binary(ss, data.aabb_min);
        to_binary(ss, data.aabb_max);
    }

    inline void from_binary(std::istringstream& ss, Mesh& data)
    {
        from_binary(ss, data.base_vertex);
        from_binary(ss, data.base_index);
        from_binary(ss, data.index_count);
        from_binary(ss, data.material_index);
        from_binary(ss, data.aabb_min);
        from_binary(ss, data.aabb_max);
    }

    inline void to_binary(std::ostringstream& ss, const ModelResource& data)
    {
        to_binary(ss, data.vertices);
        to_binary(ss, data.indices);
        to_binary(ss, data.meshes);
    }

    inline void from_binary(std::istringstream& ss, ModelResource& data)
    {
        from_binary(ss, data.vertices);
        from_binary(ss, data.indices);
        from_binary(ss, data.meshes);
    }
};  // namespace mag::fs

#include "magnolia/platform/serializer.hpp"
