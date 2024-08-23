#pragma once

#include <map>
#include <memory>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
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
            ModelManager();
            ~ModelManager();

            std::shared_ptr<Model> load(const str& file);
            b8 is_extension_supported(const str& extension_with_dot);

        private:
            void initialize_mesh(const u32 mesh_idx, const aiMesh* ai_mesh, Model* model);
            void initialize_materials(const aiScene* ai_scene, const str& file, Model* model);
            void optimize_mesh(std::vector<Vertex>& vertices, std::vector<u32>& indices, Model* model);

            const str find_texture(const aiMaterial* ai_material, aiTextureType ai_type, const str& directory) const;

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
