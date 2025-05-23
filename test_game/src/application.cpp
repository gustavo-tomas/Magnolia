#include "application.hpp"

#include <core/application.hpp>
#include <core/entry_point.hpp>
#include <core/event.hpp>
#include <gfx/gfx.hpp>
#include <gfx/types.hpp>
#include <map>
#include <project/project.hpp>
#include <resources/font.hpp>
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

        shader_resource = {};
        MAG_ASSERT(mag::resource::load("magnolia/assets/shaders/text_shader.mag.json", &shader_resource),
                   "Failed to load shader");
        text_shader = mag::gfx::create_shader(shader_resource);

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
        render_text();

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
            struct GlobalData
            {
                    mat4 view;
                    mat4 projection;
            };

            static GlobalData global_data = {};
            global_data.view = camera.get_view();
            global_data.projection = camera.get_projection();

            mag::gfx::set_uniform("u_global", &global_data);
        }

        u32 mesh_offset = 0;
        u32 material_offset = 0;
        u32 texture_offset = 0;

        static MeshData mesh_data = {};
        mesh_data.material_idx = Max_U32;

        static std::map<str, Material> materials;
        static std::map<str, Image> textures;
        static std::map<str, mag::gfx::TextureHandle> texture_handles;
        static std::map<str, mag::gfx::VertexBufferHandle> vertex_buffer_handles;
        static std::map<str, mag::gfx::IndexBufferHandle> index_buffer_handles;

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
                const mag::gfx::VertexBufferHandle vertex_buffer =
                    mag::gfx::create_vertex_buffer(VEC_SIZE_BYTES(model->vertices), model->vertices.data());

                const mag::gfx::IndexBufferHandle index_buffer =
                    mag::gfx::create_index_buffer(VEC_SIZE_BYTES(model->indices), model->indices.data());

                vertex_buffer_handles[model_name] = vertex_buffer;
                index_buffer_handles[model_name] = index_buffer;
            }

            mag::gfx::bind_vertex_buffer(vertex_buffer_handles[model_name]);
            mag::gfx::bind_index_buffer(index_buffer_handles[model_name]);

            for (auto& mesh : model->meshes)
            {
                // Instance
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
                    static MaterialData material_data = {};
                    material_data.albedo = vec4(1, 1, 1, 1);
                    material_data.roughness = 1;
                    material_data.metallic = 1;
                    material_data.albedo_tex_idx = texture_offset;
                    material_data.normal_tex_idx = texture_offset + 1;
                    material_data.roughness_tex_idx = texture_offset + 2;
                    material_data.metalness_tex_idx = texture_offset + 3;

                    mag::gfx::set_uniform("u_material", &material_data, material_offset);

                    mag::gfx::set_uniform("u_material_textures",
                                          texture_handles[material.textures.at(TextureSlot::Albedo)], texture_offset);

                    mag::gfx::set_uniform("u_material_textures",
                                          texture_handles[material.textures.at(TextureSlot::Normal)],
                                          texture_offset + 1);

                    mag::gfx::set_uniform("u_material_textures",
                                          texture_handles[material.textures.at(TextureSlot::Roughness)],
                                          texture_offset + 2);  // ARM texture

                    mag::gfx::set_uniform("u_material_textures",
                                          texture_handles[material.textures.at(TextureSlot::Metalness)],
                                          texture_offset + 2);  // ARM texture

                    material_offset++;
                    texture_offset += 4;
                }

                mag::gfx::set_uniform("u_instance", &mesh_data, mesh_offset);

                // Draw the mesh
                mag::gfx::draw_indexed(mesh.index_count, 1, mesh.base_index, mesh.base_vertex, mesh_offset);

                mesh_offset++;
            }
        }
    }

    void TestGame::render_sprites()
    {
        mag::Camera& camera = scene->get_camera();

        mag::gfx::use_shader(sprite_shader);

        // Global buffer
        {
            struct GlobalData
            {
                    mat4 view;
                    mat4 projection;
            };

            static GlobalData global_data = {};
            global_data.view = camera.get_view();
            global_data.projection = camera.get_projection();

            mag::gfx::set_uniform("u_global", &global_data);
        }

        for (u32 i = 0; i < texture_handles.size(); i++)
        {
            static SpriteData sprite_data = {};
            sprite_data.model = math::translate(mat4(1.0f), vec3(i * 100.0f, 0, 0));
            sprite_data.size_const_face = vec4(textures[i].width * 0.05f, textures[i].height * 0.05f, 0, 0);
            sprite_data.texture_idx = i;

            // Instance
            mag::gfx::set_uniform("u_instance", &sprite_data, i);

            // Textures
            mag::gfx::set_uniform("u_sprite_textures", texture_handles[i], i);

            mag::gfx::draw(4, 1, 0, i);
        }
    }

    // @TODO: temp
    struct _FontData
    {
            std::map<c8, Image> char_textures;
            std::map<c8, mag::gfx::TextureHandle> char_texture_handles;
            Font font;
            u32 idx;
    };

    static std::map<str, _FontData> fonts;

    void TestGame::render_text()
    {
        mag::gfx::use_shader(text_shader);

        mag::Camera& camera = scene->get_camera();

        struct GlobalData
        {
                mat4 view;
                mat4 projection;
        };

        static GlobalData global_data = {};
        global_data.view = camera.get_view();
        global_data.projection = camera.get_projection();

        mag::gfx::set_uniform("u_global", &global_data);

        auto text_entities = scene->get_ecs().get_all_components_of_types<TransformComponent, TextComponent>();

        u32 char_offset = 0;
        for (u32 i = 0; i < text_entities.size(); i++)
        {
            const auto& transform = std::get<0>(text_entities[i]);
            const auto& text = std::get<1>(text_entities[i]);

            const str& name = text->font->name;

            if (!fonts.contains(name))
            {
                Font font = {};
                mag::resource::load(name, &font);

                _FontData font_data = {};
                font_data.font = font;
                font_data.idx = fonts.size();  // The index is used to map a letter to the correct texture (and font)

                for (auto& [c, ch] : font.characters)
                {
                    // Skip non visual characters
                    if (ch.texture.pixels.empty())
                    {
                        continue;
                    }

                    font_data.char_texture_handles[c] =
                        mag::gfx::create_texture(ch.texture.width, ch.texture.height, ch.texture.pixels.size(),
                                                 ch.texture.pixels.data(), mag::gfx::Format::R8_UNORM);

                    font_data.char_textures[c] = ch.texture;
                }

                fonts[name] = font_data;
            }

            // Skip fonts that are not loaded yet
            if (text->font->loading_status != LoadingStatus::Finished)
            {
                continue;
            }

            const f32 scale = transform->scale.x;
            f32 x = transform->translation.x;
            f32 y = transform->translation.y;
            f32 z = transform->translation.z;

            for (auto& c : text->text)
            {
                Character& ch = text->font->characters[c];

                // Skip chars with no visual representation (i.e. spaces)
                if (ch.data.empty())
                {
                    x += (ch.advance.x >> 6) * scale;
                    continue;
                }

                // Format newlines
                if (c == '\n')
                {
                    y -= ch.size.y * 1.5 * scale;  // @TODO: hardcoded line spacing
                    x = transform->translation.x;
                    continue;
                }

                // Don't offset the first letter of the text
                const f32 xpos = x + (char_offset > 0 ? ch.bearing.x * scale : 0);
                const f32 ypos = y - (char_offset > 0 ? (ch.size.y - ch.bearing.y) * scale : 0);
                const f32 zpos = z;

                TransformComponent char_transform;
                char_transform.translation = vec3(xpos, ypos, zpos);
                char_transform.scale = vec3(ch.size.x * scale, ch.size.y * scale, 1.0f);
                char_transform.rotation = transform->rotation;

                // @TODO: rotation is a bit iffy but for now its ok
                const mat4 model_matrix = char_transform.get_transformation_matrix();

                const u32 font_offset = fonts[name].idx * 128;  // skip next 128 character textures
                const u32 texture_idx = font_offset + c;

                TextData text_data;
                text_data.color = text->color;
                text_data.model = model_matrix;
                text_data.texture_idx = texture_idx;

                mag::gfx::set_uniform("u_instance", &text_data, char_offset);

                mag::gfx::set_uniform("u_char_textures", fonts[name].char_texture_handles[c], texture_idx);

                char_offset++;

                // Advance cursors for next glyph (note that advance is number of 1/64 pixels) bitshift by 6 to get
                // value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
                x += (ch.advance.x >> 6) * scale;
            }
        }

        gfx::draw(4, char_offset);
    }
};  // namespace game
