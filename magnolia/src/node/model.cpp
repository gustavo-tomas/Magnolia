#include "node/model.hpp"

#include "renderer/context.hpp"

namespace mag
{
    Model::Model()
    {
        auto& context = get_context();

        // Create a triangle mesh
        mesh.vertices.resize(3);
        mesh.vertices[0].position = {0.5f, 0.5f, 0.0f};
        mesh.vertices[1].position = {-0.5f, 0.5f, 0.0f};
        mesh.vertices[2].position = {0.0f, -0.5f, 0.0f};

        mesh.vertices[0].normal = {1.0f, 0.0f, 0.0f};
        mesh.vertices[1].normal = {0.0f, 1.0f, 0.0f};
        mesh.vertices[2].normal = {0.0f, 0.0f, 1.0f};

        mesh.vbo.initialize(mesh.vertices.data(), VECSIZE(mesh.vertices) * sizeof(Vertex), context.get_allocator());
    }

    Model::~Model()
    {
        get_context().get_device().waitIdle();

        mesh.vbo.shutdown();
    }
};  // namespace mag
