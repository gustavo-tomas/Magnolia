#pragma once

#include <memory>
#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "core/types.hpp"

namespace mag
{
    using namespace math;

    struct Model;
    struct Vertex;

    class ModelLoader
    {
        public:
            ModelLoader();

            b8 load(const str& file_path, Model* model);
            b8 is_extension_supported(const str& extension_with_dot);

        private:
            b8 import_from_file(const str& name, Model* model);
            b8 load_native(const str& file_path, Model* model);
            void create_native_file(const str& output_directory, Model* model_resource);

            void initialize_mesh(const u32 mesh_idx, const aiMesh* ai_mesh, Model* model_resource);
            void initialize_materials(const aiScene* ai_scene, const str& file_path, const str& output_directory,
                                      Model* model_resource);
            void optimize_mesh(std::vector<Vertex>& vertices, std::vector<u32>& indices, Model* model_resource);

            const str find_texture(const aiMaterial* ai_material, aiTextureType ai_type, const str& directory) const;

            std::unique_ptr<Assimp::Importer> importer;
    };
};  // namespace mag
