#pragma once

#include <assimp/Importer.hpp>
#include <map>
#include <memory>
#include <vector>

#include "renderer/buffers.hpp"
#include "renderer/image.hpp"

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
            std::vector<std::shared_ptr<Image>> textures;
    };

    struct Model
    {
            vec3 position = vec3(0.0f);
            vec3 rotation = vec3(0.0f);
            vec3 scale = vec3(1.0f);

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

    class ModelLoader : public Singleton
    {
        public:
            virtual void initialize() override;
            virtual void shutdown() override;

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
            void initialize();
            void shutdown();

            Model& get_model() { return model; };

        private:
            Model model;
    };
};  // namespace mag
