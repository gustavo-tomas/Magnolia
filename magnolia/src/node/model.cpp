#include "node/model.hpp"

#include "renderer/context.hpp"

namespace mag
{
    Model::Model()
    {
        auto& context = get_context();

        // Create a cube mesh
        mesh.vertices.resize(8);

        mesh.vertices[0].position = {-1.0f, -1.0f, 1.0f};
        mesh.vertices[1].position = {1.0f, -1.0f, 1.0f};
        mesh.vertices[2].position = {1.0f, 1.0f, 1.0f};
        mesh.vertices[3].position = {-1.0f, 1.0f, 1.0f};

        mesh.vertices[4].position = {-1.0f, -1.0f, -1.0f};
        mesh.vertices[5].position = {1.0f, -1.0f, -1.0f};
        mesh.vertices[6].position = {1.0f, 1.0f, -1.0f};
        mesh.vertices[7].position = {-1.0f, 1.0f, -1.0f};

        mesh.vertices[0].normal = normalize(mesh.vertices[0].position);
        mesh.vertices[1].normal = normalize(mesh.vertices[1].position);
        mesh.vertices[2].normal = normalize(mesh.vertices[2].position);
        mesh.vertices[3].normal = normalize(mesh.vertices[3].position);

        mesh.vertices[4].normal = normalize(mesh.vertices[4].position);
        mesh.vertices[5].normal = normalize(mesh.vertices[5].position);
        mesh.vertices[6].normal = normalize(mesh.vertices[6].position);
        mesh.vertices[7].normal = normalize(mesh.vertices[7].position);

        mesh.indices = {// Front face
                        0, 1, 2, 2, 3, 0,
                        // Back face
                        4, 5, 6, 6, 7, 4,
                        // Left face
                        0, 3, 7, 7, 4, 0,
                        // Right face
                        1, 2, 6, 6, 5, 1,
                        // Top face
                        3, 2, 6, 6, 7, 3,
                        // Bottom face
                        0, 1, 5, 5, 4, 0};

        mesh.vbo.initialize(mesh.vertices.data(), VECSIZE(mesh.vertices) * sizeof(Vertex), context.get_allocator());
        mesh.ibo.initialize(mesh.indices.data(), VECSIZE(mesh.indices) * sizeof(u32), context.get_allocator());
    }

    Model::~Model()
    {
        get_context().get_device().waitIdle();

        mesh.vbo.shutdown();
        mesh.ibo.shutdown();
    }
};  // namespace mag
