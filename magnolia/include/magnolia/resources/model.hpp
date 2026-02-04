#pragma once

#include <vector>

#include "magnolia/core/types.hpp"
#include "magnolia/math/types.hpp"
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
