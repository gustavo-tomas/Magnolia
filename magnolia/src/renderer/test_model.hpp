#pragma once

#include <vector>

#include "renderer/buffers.hpp"
#include "resources/model.hpp"

namespace mag
{
    using namespace math;

    // @TODO: testing
    class Cube
    {
        public:
            Cube(const str& name = "Cube");

            Model& get_model() { return model; };

        private:
            Model model;
    };

    struct QuadVertex
    {
            vec3 positon;
            vec2 tex_coords;
    };

    class Quad
    {
        public:
            Quad(const vec2 dimensions = vec2(1.0f, 1.0f));

            VertexBuffer& get_vbo() { return *vbo; };
            IndexBuffer& get_ibo() { return *ibo; };

            const std::vector<QuadVertex>& get_vertices() const { return vertices; };
            const std::vector<u32>& get_indices() const { return indices; };

        private:
            unique<VertexBuffer> vbo;
            unique<IndexBuffer> ibo;

            std::vector<QuadVertex> vertices;
            std::vector<u32> indices;
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

            VertexBuffer& get_vbo() { return *vbo; };
            const std::vector<LineVertex>& get_vertices() const { return vertices; };

        private:
            unique<VertexBuffer> vbo;
            std::vector<LineVertex> vertices;
    };
    // @TODO: testing
};  // namespace mag
