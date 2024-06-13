#pragma once

#include <map>
#include <memory>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "renderer/buffers.hpp"
#include "renderer/material.hpp"

namespace mag
{
    struct Vertex
    {
            math::vec3 position;
            math::vec3 normal;
            math::vec2 tex_coords;
            math::vec3 tangent;
            math::vec3 bitangent;
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
            str name;
            std::vector<Mesh> meshes;

            std::vector<std::shared_ptr<Material>> materials;
            std::vector<Vertex> vertices;
            std::vector<u32> indices;

            VertexBuffer vbo;
            IndexBuffer ibo;
    };

    class ModelManager
    {
        public:
            ModelManager();
            ~ModelManager();

            std::shared_ptr<Model> load(const str& file);
            b8 is_extension_supported(const str& extension_with_dot);

        private:
            void initialize_mesh(const u32 mesh_idx, const aiMesh* ai_mesh, Model* model);
            void initialize_materials(const aiScene* ai_scene, const str& file, Model* model);
            void optimize_mesh(std::vector<Vertex>& vertices, std::vector<u32>& indices, Model* model);

            std::shared_ptr<Image> load_texture(const aiMaterial* ai_material, aiTextureType ai_type,
                                                const str& directory);

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

    class Line
    {
        public:
            Line(const str& name, const std::vector<vec3>& starts, const std::vector<vec3>& ends,
                 const std::vector<vec3>& colors);
            ~Line();

            Model& get_model() { return model; };

        private:
            Model model;
    };
    // @TODO: testing
};  // namespace mag
