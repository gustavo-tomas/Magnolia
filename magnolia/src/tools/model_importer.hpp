#pragma once

#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "core/types.hpp"

namespace mag
{
    struct Model;
    struct Vertex;

    class ModelImporter
    {
        public:
            ModelImporter();

            b8 import(const str& name, str& imported_model_path);
            b8 is_extension_supported(const str& extension_with_dot);

        private:
            b8 create_native_file(const str& output_directory, const Model& model, str& imported_model_path);

            b8 initialize_mesh(const u32 mesh_idx, const aiMesh* ai_mesh, Model& model);
            void initialize_materials(const aiScene* ai_scene, const str& file_path, const str& output_directory,
                                      Model& model);
            void optimize_mesh(std::vector<Vertex>& vertices, std::vector<u32>& indices, Model& model);

            const str find_texture(const aiMaterial* ai_material, aiTextureType ai_type, const str& directory) const;

            unique<Assimp::Importer> importer;
    };
};  // namespace mag
