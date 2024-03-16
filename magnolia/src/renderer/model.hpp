#pragma once

#include <assimp/Importer.hpp>
#include <map>
#include <memory>

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
            math::vec2 tex_coords;
    };

    struct Mesh
    {
            VertexBuffer vbo;
            IndexBuffer ibo;
            std::vector<Vertex> vertices;
            std::vector<u32> indices;
    };

    struct Model
    {
            std::vector<Mesh> meshes;
            str name;
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

        vk::VertexInputAttributeDescription tex_coords_attribute(location++, 0, vk::Format::eR32G32Sfloat,
                                                                 offsetof(Vertex, tex_coords));
        description.attributes.push_back(tex_coords_attribute);

        return description;
    }

    class ModelLoader
    {
        public:
            void initialize();
            void shutdown();

            std::shared_ptr<Model> load(const str& file);

        private:
            std::unique_ptr<Assimp::Importer> importer;
            std::map<str, std::shared_ptr<Model>> models;
    };

    // @TODO: testing
    class Cube
    {
        public:
            inline void initialize()
            {
                mesh.vertices.resize(24);

                // Positions for each vertex
                mesh.vertices[0].position = {-1.0f, -1.0f, 1.0f};  // Front bottom-left
                mesh.vertices[1].position = {1.0f, -1.0f, 1.0f};   // Front bottom-right
                mesh.vertices[2].position = {1.0f, 1.0f, 1.0f};    // Front top-right
                mesh.vertices[3].position = {-1.0f, 1.0f, 1.0f};   // Front top-left

                mesh.vertices[4].position = {-1.0f, -1.0f, -1.0f};  // Back bottom-left
                mesh.vertices[5].position = {1.0f, -1.0f, -1.0f};   // Back bottom-right
                mesh.vertices[6].position = {1.0f, 1.0f, -1.0f};    // Back top-right
                mesh.vertices[7].position = {-1.0f, 1.0f, -1.0f};   // Back top-left

                mesh.vertices[8].position = {-1.0f, -1.0f, 1.0f};   // Left bottom-front
                mesh.vertices[9].position = {-1.0f, -1.0f, -1.0f};  // Left bottom-back
                mesh.vertices[10].position = {-1.0f, 1.0f, -1.0f};  // Left top-back
                mesh.vertices[11].position = {-1.0f, 1.0f, 1.0f};   // Left top-front

                mesh.vertices[12].position = {1.0f, -1.0f, 1.0f};   // Right bottom-front
                mesh.vertices[13].position = {1.0f, -1.0f, -1.0f};  // Right bottom-back
                mesh.vertices[14].position = {1.0f, 1.0f, -1.0f};   // Right top-back
                mesh.vertices[15].position = {1.0f, 1.0f, 1.0f};    // Right top-front

                mesh.vertices[16].position = {-1.0f, 1.0f, 1.0f};   // Top front-left
                mesh.vertices[17].position = {1.0f, 1.0f, 1.0f};    // Top front-right
                mesh.vertices[18].position = {1.0f, 1.0f, -1.0f};   // Top back-right
                mesh.vertices[19].position = {-1.0f, 1.0f, -1.0f};  // Top back-left

                mesh.vertices[20].position = {-1.0f, -1.0f, 1.0f};   // Bottom front-left
                mesh.vertices[21].position = {1.0f, -1.0f, 1.0f};    // Bottom front-right
                mesh.vertices[22].position = {1.0f, -1.0f, -1.0f};   // Bottom back-right
                mesh.vertices[23].position = {-1.0f, -1.0f, -1.0f};  // Bottom back-left

                // Front face
                mesh.vertices[0].tex_coords = {0.0f, 0.0f};
                mesh.vertices[1].tex_coords = {1.0f, 0.0f};
                mesh.vertices[2].tex_coords = {1.0f, 1.0f};
                mesh.vertices[3].tex_coords = {0.0f, 1.0f};

                // Back face
                mesh.vertices[4].tex_coords = {0.0f, 0.0f};
                mesh.vertices[5].tex_coords = {1.0f, 0.0f};
                mesh.vertices[6].tex_coords = {1.0f, 1.0f};
                mesh.vertices[7].tex_coords = {0.0f, 1.0f};

                // Left face
                mesh.vertices[8].tex_coords = {0.0f, 0.0f};
                mesh.vertices[9].tex_coords = {1.0f, 0.0f};
                mesh.vertices[10].tex_coords = {1.0f, 1.0f};
                mesh.vertices[11].tex_coords = {0.0f, 1.0f};

                // Right face
                mesh.vertices[12].tex_coords = {0.0f, 0.0f};
                mesh.vertices[13].tex_coords = {1.0f, 0.0f};
                mesh.vertices[14].tex_coords = {1.0f, 1.0f};
                mesh.vertices[15].tex_coords = {0.0f, 1.0f};

                // Top face
                mesh.vertices[16].tex_coords = {0.0f, 0.0f};
                mesh.vertices[17].tex_coords = {1.0f, 0.0f};
                mesh.vertices[18].tex_coords = {1.0f, 1.0f};
                mesh.vertices[19].tex_coords = {0.0f, 1.0f};

                // Bottom face
                mesh.vertices[20].tex_coords = {0.0f, 0.0f};
                mesh.vertices[21].tex_coords = {1.0f, 0.0f};
                mesh.vertices[22].tex_coords = {1.0f, 1.0f};
                mesh.vertices[23].tex_coords = {0.0f, 1.0f};

                // Indices for the cube
                mesh.indices = {// Front face
                                0, 1, 2, 2, 3, 0,
                                // Back face
                                4, 5, 6, 6, 7, 4,
                                // Left face
                                8, 9, 10, 10, 11, 8,
                                // Right face
                                12, 13, 14, 14, 15, 12,
                                // Top face
                                16, 17, 18, 18, 19, 16,
                                // Bottom face
                                20, 21, 22, 22, 23, 20};

                mesh.vbo.initialize(mesh.vertices.data(), VECSIZE(mesh.vertices) * sizeof(Vertex));
                mesh.ibo.initialize(mesh.indices.data(), VECSIZE(mesh.indices) * sizeof(u32));
            }

            inline void shutdown()
            {
                mesh.vbo.shutdown();
                mesh.ibo.shutdown();
            }

            const Mesh& get_mesh() const { return mesh; };

        private:
            Mesh mesh;
    };
};  // namespace mag
