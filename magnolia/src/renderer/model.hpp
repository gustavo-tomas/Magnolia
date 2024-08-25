#pragma once

#include <map>
#include <memory>
#include <vector>

#include "renderer/buffers.hpp"

namespace mag
{
    using namespace math;

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
    };

    struct Model
    {
            str name = "";
            str file_path = "";
            std::vector<Mesh> meshes;

            std::vector<str> materials;
            std::vector<Vertex> vertices;
            std::vector<u32> indices;

            VertexBuffer vbo;
            IndexBuffer ibo;
    };

    class ModelManager
    {
        public:
            ~ModelManager();

            std::shared_ptr<Model> get(const str& name);

        private:
            std::map<str, std::shared_ptr<Model>> models;
    };

    // @TODO: testing
    class Cube
    {
        public:
            Cube(const str& name = "Cube");
            ~Cube();

            Model& get_model() { return model; };

        private:
            Model model;
    };

    struct LineVertex
    {
            vec3 position;
            vec3 color;
    };

    class Line
    {
        public:
            Line(const std::vector<vec3>& starts, const std::vector<vec3>& ends, const std::vector<vec3>& colors);
            ~Line();

            const VertexBuffer& get_vbo() const { return vbo; };
            const std::vector<LineVertex>& get_vertices() const { return vertices; };

        private:
            VertexBuffer vbo;
            std::vector<LineVertex> vertices;
    };
    // @TODO: testing
};  // namespace mag
