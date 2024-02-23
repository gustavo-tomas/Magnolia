#pragma once

#include "renderer/buffers.hpp"

namespace mag
{
    struct VertexInputDescription
    {
            std::vector<vk::VertexInputBindingDescription> bindings;
            std::vector<vk::VertexInputAttributeDescription> attributes;
    };

    struct Vertex
    {
            static VertexInputDescription get_vertex_description();

            math::vec3 position;
            math::vec3 normal;
    };

    struct Mesh
    {
            VertexBuffer vbo;
            std::vector<Vertex> vertices;
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
