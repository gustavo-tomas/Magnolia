#pragma once

#include "node/node.hpp"
#include "renderer/buffers.hpp"

// @TODO: encapsulate this pls tyty

namespace mag
{
    using namespace math;

    struct VertexInputDescription
    {
            std::vector<vk::VertexInputBindingDescription> bindings;
            std::vector<vk::VertexInputAttributeDescription> attributes;
    };

    struct Vertex
    {
            static VertexInputDescription get_vertex_description();

            vec3 position;
            vec3 normal;
    };

    struct Mesh
    {
            VertexBuffer vbo;
            IndexBuffer ibo;
            std::vector<Vertex> vertices;
            std::vector<u32> indices;
    };

    class Model : public Node
    {
        public:
            Model();
            ~Model();

            void set_position(const vec3& position);

            const Mesh& get_mesh() const { return mesh; };
            const mat4& get_model_matrix() const { return model_matrix; };

        private:
            Mesh mesh;
            mat4 model_matrix = mat4(1.0f);
    };

    inline VertexInputDescription Vertex::get_vertex_description()
    {
        VertexInputDescription description = {};

        u32 location = 0;

        vk::VertexInputBindingDescription binding_description(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
        description.bindings.push_back(binding_description);

        vk::VertexInputAttributeDescription position_attribute(location++, 0, vk::Format::eR32G32B32Sfloat,
                                                               offsetof(Vertex, position));
        description.attributes.push_back(position_attribute);

        vk::VertexInputAttributeDescription normal_attribute(location++, 0, vk::Format::eR32G32B32Sfloat,
                                                             offsetof(Vertex, normal));
        description.attributes.push_back(normal_attribute);

        return description;
    }
};  // namespace mag
