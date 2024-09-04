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

    struct LineVertex
    {
            vec3 position;
            vec3 color;
    };

    class Line
    {
        public:
            Line(const std::vector<vec3>& starts, const std::vector<vec3>& ends, const std::vector<vec3>& colors);

            const VertexBuffer& get_vbo() const { return *vbo; };
            const std::vector<LineVertex>& get_vertices() const { return vertices; };

        private:
            std::unique_ptr<VertexBuffer> vbo;
            std::vector<LineVertex> vertices;
    };
    // @TODO: testing
};  // namespace mag
