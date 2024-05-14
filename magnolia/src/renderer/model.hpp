#pragma once

#include <assimp/Importer.hpp>
#include <map>
#include <memory>
#include <vector>

#include "renderer/buffers.hpp"
#include "renderer/image.hpp"

namespace mag
{
    struct Vertex
    {
            math::vec3 position;
            math::vec3 normal;
            math::vec2 tex_coords;
    };

    struct Mesh
    {
            str name;
            VertexBuffer vbo;
            IndexBuffer ibo;
            std::vector<Vertex> vertices;
            std::vector<u32> indices;
            std::vector<std::shared_ptr<Image>> textures;
    };

    struct Model
    {
            std::vector<Mesh> meshes;
            str name;
    };

    class ModelLoader
    {
        public:
            ModelLoader();
            ~ModelLoader();

            std::shared_ptr<Model> load(const str& file);
            b8 is_extension_supported(const str& extension_with_dot);

        private:
            std::unique_ptr<Assimp::Importer> importer;
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
};  // namespace mag
