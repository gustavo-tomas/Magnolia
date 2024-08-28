#include "renderer/test_model.hpp"

#include "core/application.hpp"
#include "core/logger.hpp"
#include "renderer/context.hpp"

namespace mag
{
    Cube::Cube(const str& name)
    {
        model.name = name;
        model.meshes.resize(1);
        model.meshes[0].base_index = 0;
        model.meshes[0].base_vertex = 0;
        model.meshes[0].material_index = 0;

        model.vertices.resize(24);

        // Positions for each vertex
        model.vertices[0].position = {-1.0f, -1.0f, 1.0f};  // Front bottom-left
        model.vertices[1].position = {1.0f, -1.0f, 1.0f};   // Front bottom-right
        model.vertices[2].position = {1.0f, 1.0f, 1.0f};    // Front top-right
        model.vertices[3].position = {-1.0f, 1.0f, 1.0f};   // Front top-left

        model.vertices[4].position = {-1.0f, -1.0f, -1.0f};  // Back bottom-left
        model.vertices[5].position = {1.0f, -1.0f, -1.0f};   // Back bottom-right
        model.vertices[6].position = {1.0f, 1.0f, -1.0f};    // Back top-right
        model.vertices[7].position = {-1.0f, 1.0f, -1.0f};   // Back top-left

        model.vertices[8].position = {-1.0f, -1.0f, 1.0f};   // Left bottom-front
        model.vertices[9].position = {-1.0f, -1.0f, -1.0f};  // Left bottom-back
        model.vertices[10].position = {-1.0f, 1.0f, -1.0f};  // Left top-back
        model.vertices[11].position = {-1.0f, 1.0f, 1.0f};   // Left top-front

        model.vertices[12].position = {1.0f, -1.0f, 1.0f};   // Right bottom-front
        model.vertices[13].position = {1.0f, -1.0f, -1.0f};  // Right bottom-back
        model.vertices[14].position = {1.0f, 1.0f, -1.0f};   // Right top-back
        model.vertices[15].position = {1.0f, 1.0f, 1.0f};    // Right top-front

        model.vertices[16].position = {-1.0f, 1.0f, 1.0f};   // Top front-left
        model.vertices[17].position = {1.0f, 1.0f, 1.0f};    // Top front-right
        model.vertices[18].position = {1.0f, 1.0f, -1.0f};   // Top back-right
        model.vertices[19].position = {-1.0f, 1.0f, -1.0f};  // Top back-left

        model.vertices[20].position = {-1.0f, -1.0f, 1.0f};   // Bottom front-left
        model.vertices[21].position = {1.0f, -1.0f, 1.0f};    // Bottom front-right
        model.vertices[22].position = {1.0f, -1.0f, -1.0f};   // Bottom back-right
        model.vertices[23].position = {-1.0f, -1.0f, -1.0f};  // Bottom back-left

        for (auto& vertex : model.vertices) vertex.normal = normalize(vertex.position);

        // Front face
        model.vertices[0].tex_coords = {0.0f, 0.0f};
        model.vertices[1].tex_coords = {1.0f, 0.0f};
        model.vertices[2].tex_coords = {1.0f, 1.0f};
        model.vertices[3].tex_coords = {0.0f, 1.0f};

        // Back face
        model.vertices[4].tex_coords = {0.0f, 0.0f};
        model.vertices[5].tex_coords = {1.0f, 0.0f};
        model.vertices[6].tex_coords = {1.0f, 1.0f};
        model.vertices[7].tex_coords = {0.0f, 1.0f};

        // Left face
        model.vertices[8].tex_coords = {0.0f, 0.0f};
        model.vertices[9].tex_coords = {1.0f, 0.0f};
        model.vertices[10].tex_coords = {1.0f, 1.0f};
        model.vertices[11].tex_coords = {0.0f, 1.0f};

        // Right face
        model.vertices[12].tex_coords = {0.0f, 0.0f};
        model.vertices[13].tex_coords = {1.0f, 0.0f};
        model.vertices[14].tex_coords = {1.0f, 1.0f};
        model.vertices[15].tex_coords = {0.0f, 1.0f};

        // Top face
        model.vertices[16].tex_coords = {0.0f, 0.0f};
        model.vertices[17].tex_coords = {1.0f, 0.0f};
        model.vertices[18].tex_coords = {1.0f, 1.0f};
        model.vertices[19].tex_coords = {0.0f, 1.0f};

        // Bottom face
        model.vertices[20].tex_coords = {0.0f, 0.0f};
        model.vertices[21].tex_coords = {1.0f, 0.0f};
        model.vertices[22].tex_coords = {1.0f, 1.0f};
        model.vertices[23].tex_coords = {0.0f, 1.0f};

        // Tangents and Bitangents
        for (u32 i = 0; i < model.vertices.size(); i += 3)
        {
            Vertex& v0 = model.vertices[i];
            Vertex& v1 = model.vertices[i + 1];
            Vertex& v2 = model.vertices[i + 2];

            const vec3 edge_1 = v1.position - v0.position;
            const vec3 edge_2 = v2.position - v0.position;

            const vec2 delta_uv1 = v1.tex_coords - v0.tex_coords;
            const vec2 delta_uv2 = v2.tex_coords - v0.tex_coords;

            const f32 f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

            vec3 tangent;
            tangent.x = f * (delta_uv2.y * edge_1.x - delta_uv1.y * edge_2.x);
            tangent.y = f * (delta_uv2.y * edge_1.y - delta_uv1.y * edge_2.y);
            tangent.z = f * (delta_uv2.y * edge_1.z - delta_uv1.y * edge_2.z);

            vec3 bitangent;
            bitangent.x = f * (-delta_uv2.x * edge_1.x + delta_uv1.x * edge_2.x);
            bitangent.y = f * (-delta_uv2.x * edge_1.y + delta_uv1.x * edge_2.y);
            bitangent.z = f * (-delta_uv2.x * edge_1.z + delta_uv1.x * edge_2.z);

            v0.tangent = v1.tangent = v2.tangent = normalize(tangent);
            v0.bitangent = v1.bitangent = v2.bitangent = normalize(bitangent);
        }

        // Indices for the cube
        model.indices = {// Front face
                         0, 1, 2, 2, 3, 0,
                         // Back face
                         6, 5, 4, 7, 6, 4,
                         // Left face
                         10, 9, 8, 11, 10, 8,
                         // Right face
                         12, 13, 14, 14, 15, 12,
                         // Top face
                         16, 17, 18, 18, 19, 16,
                         // Bottom face
                         22, 21, 20, 23, 22, 20};

        model.meshes[0].index_count = model.indices.size();

        // Use the default material
        model.materials.push_back(DEFAULT_MATERIAL_NAME);

        // Send model data to the GPU
        auto& app = get_application();
        auto& renderer = app.get_renderer();

        renderer.add_model(&model);
    }

    Line::Line(const std::vector<vec3>& starts, const std::vector<vec3>& ends, const std::vector<vec3>& colors)
    {
        for (u32 i = 0; i < starts.size(); i++)
        {
            LineVertex line_start;
            LineVertex line_end;

            line_start.position = starts[i];
            line_end.position = ends[i];

            // We use the models normal as the color
            line_start.color = colors[i];
            line_end.color = colors[i];

            vertices.push_back(line_start);
            vertices.push_back(line_end);
        }

        vbo.initialize(vertices.data(), VEC_SIZE_BYTES(vertices));
    }

    Line::~Line()
    {
        get_context().get_device().waitIdle();

        vbo.shutdown();
    }
};  // namespace mag
