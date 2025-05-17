#include "application.hpp"

#include <core/application.hpp>
#include <core/entry_point.hpp>
#include <core/event.hpp>
#include <gfx/gfx.hpp>
#include <map>
#include <project/project.hpp>
#include <resources/image.hpp>
#include <resources/material.hpp>
#include <resources/model.hpp>
#include <resources/resource_loader.hpp>
#include <resources/shader.hpp>
#include <scene/scene.hpp>
#include <scene/scene_serializer.hpp>

// @TODO: temp
#include "../magnolia/assets/shaders/include/common.h"

mag::Application* mag::create_application() { return new game::TestGame("test_game/config.json"); }

namespace game
{
    // @TODO: temp
    static std::vector<mag::gfx::TextureHandle> texture_handles;
    static std::vector<mag::Image> textures;

    TestGame::TestGame(const str& config_file_path) : Application(config_file_path)
    {
        // Load the project

        mag::Project project;

        const str project_file_path = "test_game/TestGame.proj.json";
        if (!mag::project::load(project_file_path, project))
        {
            LOG_ERROR("Failed to load project: '{0}'", project_file_path);
            return;
        }

        // @TODO: temp - load shaders

        mag::ShaderResource shader_resource = {};
        MAG_ASSERT(mag::resource::load("magnolia/assets/shaders/sprite_shader.mag.json", &shader_resource),
                   "Failed to load shader");
        sprite_shader = mag::gfx::create_shader(shader_resource);

        shader_resource = {};
        MAG_ASSERT(mag::resource::load("magnolia/assets/shaders/mesh_shader.mag.json", &shader_resource),
                   "Failed to load shader");
        mesh_shader = mag::gfx::create_shader(shader_resource);

        // Then load starting scene

        scene = create_unique<mag::Scene>();

        const str start_scene_file_path = project.get_asset_dir() / project.get_relative_start_scene_path();
        if (!mag::scene::load(start_scene_file_path, *scene))
        {
            LOG_ERROR("Failed to load start scene: '{0}'", start_scene_file_path);
            return;
        }

        scene->on_start();

        u32 texture_count = 2;

        textures.resize(texture_count);

        for (u32 i = 0; i < texture_count; i++)
        {
            MAG_ASSERT(mag::resource::load("magnolia/assets/test_texture" + std::to_string(i) + ".png", &textures[i]),
                       "Failed to load textures[{0}]", i);
            texture_handles.push_back(mag::gfx::create_texture(textures[i].width, textures[i].height,
                                                               textures[i].pixels.size(), textures[i].pixels.data()));
        }
    }

    TestGame::~TestGame() = default;

    void TestGame::on_update(const f32 dt)
    {
        // Check for scene swaps
        const str& next_scene_file_path = scene->get_next_scene();
        if (!next_scene_file_path.empty())
        {
            mag::Scene* next_scene = new mag::Scene();
            if (!mag::scene::load(next_scene_file_path, *next_scene))
            {
                LOG_ERROR("Failed to load scene: '{0}'", next_scene_file_path);
                delete next_scene;
            }

            else
            {
                scene.reset(next_scene);
                scene->on_start();
            }

            scene->set_next_scene("");
        }

        scene->on_update(dt);

        mag::gfx::begin_frame();

        render_sprites();
        render_models();

        mag::gfx::end_frame();
    }

    void TestGame::on_event(const mag::Event& e) { scene->on_event(e); }

    void TestGame::render_models()
    {
        auto& ecs = scene->get_ecs();
        const auto& camera = scene->get_camera();

        auto model_entities = ecs.get_all_components_of_types<TransformComponent, ModelComponent>();
        auto light_entities = ecs.get_all_components_of_types<TransformComponent, LightComponent>();
        auto sprite_entities = ecs.get_all_components_of_types<TransformComponent, SpriteComponent>();

        // Render models

        mag::gfx::use_shader(mesh_shader);

        // Global buffer
        {
            struct GlobalBuffer
            {
                    mat4 view;
                    mat4 projection;
            };

            static GlobalBuffer global_buffer = {};

            global_buffer.view = camera.get_view();
            global_buffer.projection = camera.get_projection();

            static mag::gfx::BufferHandle global_buffer_h = mag::gfx::create_buffer(sizeof(GlobalBuffer));
            mag::gfx::set_buffer_data(global_buffer_h, &global_buffer, sizeof(GlobalBuffer));
            mag::gfx::set_shader_buffer_uniform(mesh_shader, global_buffer_h, 0);
        }

        u32 mesh_offset = 0;
        u32 material_offset = 0;
        u32 texture_offset = 0;

        static MeshData mesh_data = {};
        mesh_data.material_idx = Max_U32;

        static std::map<str, Material> materials;
        static std::map<str, Image> textures;
        static std::map<str, mag::gfx::TextureHandle> texture_handles;
        static std::map<str, mag::gfx::BufferHandle> vertex_buffer_handles;
        static std::map<str, mag::gfx::BufferHandle> index_buffer_handles;

        for (u32 i = 0; i < model_entities.size(); i++)
        {
            const auto& transform = std::get<0>(model_entities[i]);
            const auto& model = std::get<1>(model_entities[i])->model;

            if (model->loading_status != LoadingStatus::Finished)
            {
                continue;
            }

            const str& model_name = model->name;
            if (!vertex_buffer_handles.contains(model_name))
            {
                const mag::gfx::BufferHandle vertex_buffer =
                    mag::gfx::create_buffer(VEC_SIZE_BYTES(model->vertices), model->vertices.data());

                const mag::gfx::BufferHandle index_buffer =
                    mag::gfx::create_buffer(VEC_SIZE_BYTES(model->indices), model->indices.data());

                vertex_buffer_handles[model_name] = vertex_buffer;
                index_buffer_handles[model_name] = index_buffer;
            }

            mag::gfx::bind_vertex_buffer(vertex_buffer_handles[model_name]);
            mag::gfx::bind_index_buffer(index_buffer_handles[model_name]);

            for (auto& mesh : model->meshes)
            {
                // Instance buffer
                static mag::gfx::BufferHandle mesh_buffer_h = mag::gfx::create_buffer(sizeof(MeshData) * 1000);

                mesh_data.model = transform->get_transformation_matrix();

                // Set the material. The meshes are sorted by material index (see model loader), so we draw all meshes
                // with the same material before swapping to the next one.
                if (mesh_data.material_idx != mesh.material_index)
                {
                    mesh_data.material_idx = mesh.material_index;

                    const str& material_name = model->materials[mesh.material_index];
                    if (!materials.contains(material_name))
                    {
                        Material material = {};
                        mag::resource::load(material_name, &material);

                        materials[material_name] = material;

                        for (auto& [slot, name] : material.textures)
                        {
                            if (!textures.contains(name))
                            {
                                Image texture = {};
                                mag::resource::load(name, &texture);

                                texture_handles[name] = mag::gfx::create_texture(
                                    texture.width, texture.height, texture.pixels.size(), texture.pixels.data());

                                textures[name] = texture;
                            }
                        }
                    }

                    const mag::Material& material = materials[material_name];

                    // @TODO: hardcoded material parameters
                    static MaterialData material_data;
                    material_data.albedo = vec4(1, 1, 1, 1);
                    material_data.roughness = 1;
                    material_data.metallic = 1;
                    material_data.albedo_tex_idx = texture_offset;
                    material_data.normal_tex_idx = texture_offset + 1;
                    material_data.roughness_tex_idx = texture_offset + 2;
                    material_data.metalness_tex_idx = texture_offset + 3;

                    static mag::gfx::BufferHandle material_buffer_h =
                        mag::gfx::create_buffer(sizeof(MaterialData) * 1000);

                    mag::gfx::set_buffer_data(material_buffer_h, &material_data, sizeof(MaterialData),
                                              sizeof(MaterialData) * material_offset);

                    mag::gfx::set_shader_buffer_uniform(mesh_shader, material_buffer_h, 2, material_offset);

                    mag::gfx::set_shader_texture_uniform(
                        mesh_shader, texture_handles[material.textures.at(TextureSlot::Albedo)], 3, texture_offset);

                    mag::gfx::set_shader_texture_uniform(
                        mesh_shader, texture_handles[material.textures.at(TextureSlot::Normal)], 3, texture_offset + 1);

                    mag::gfx::set_shader_texture_uniform(mesh_shader,
                                                         texture_handles[material.textures.at(TextureSlot::Roughness)],
                                                         3, texture_offset + 2);  // ARM texture

                    mag::gfx::set_shader_texture_uniform(mesh_shader,
                                                         texture_handles[material.textures.at(TextureSlot::Metalness)],
                                                         3, texture_offset + 2);  // ARM texture

                    material_offset++;
                    texture_offset += 4;
                }

                mag::gfx::set_buffer_data(mesh_buffer_h, &mesh_data, sizeof(MeshData), sizeof(MeshData) * mesh_offset);
                mag::gfx::set_shader_buffer_uniform(mesh_shader, mesh_buffer_h, 1);

                // Draw the mesh
                mag::gfx::draw_indexed(mesh.index_count, 1, mesh.base_index, mesh.base_vertex, i);

                mesh_offset++;
            }
        }
    }

    void TestGame::render_sprites()
    {
        mag::Camera& cam = scene->get_camera();

        mag::gfx::use_shader(sprite_shader);

        // Global buffer
        {
            struct alignas(16) GlobalBuffer
            {
                    mat4 view;
                    mat4 projection;
            };

            static GlobalBuffer global_buffer = {};

            global_buffer.view = cam.get_view();
            global_buffer.projection = cam.get_projection();

            static mag::gfx::BufferHandle buf_handle = mag::gfx::create_buffer(sizeof(GlobalBuffer), &global_buffer);
            mag::gfx::set_buffer_data(buf_handle, &global_buffer, sizeof(GlobalBuffer));
            mag::gfx::set_shader_buffer_uniform(sprite_shader, buf_handle, 0);
        }

        for (u32 i = 0; i < texture_handles.size(); i++)
        {
            static SpriteData sprite_data = {};
            sprite_data.model = math::translate(mat4(1.0f), vec3(i * 100.0f, 0, 0));
            sprite_data.size_const_face = vec4(textures[i].width * 0.05f, textures[i].height * 0.05f, 0, 0);
            sprite_data.texture_idx = i;

            // Instance buffer
            static mag::gfx::BufferHandle buf_handle_sprite = mag::gfx::create_buffer(sizeof(SpriteData) * 2);
            mag::gfx::set_buffer_data(buf_handle_sprite, &sprite_data, sizeof(SpriteData), sizeof(SpriteData) * i);
            mag::gfx::set_shader_buffer_uniform(sprite_shader, buf_handle_sprite, 1, i);

            // Textures
            mag::gfx::set_shader_texture_uniform(sprite_shader, texture_handles[i], 2, i);

            mag::gfx::draw(4, 1, 0, i);
        }
    }
};  // namespace game
