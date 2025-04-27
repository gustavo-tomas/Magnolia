#include "renderer/renderer.hpp"

#include <map>
#include <vulkan/vulkan.hpp>

#include "core/assert.hpp"
#include "core/event.hpp"
#include "core/logger.hpp"
#include "core/window.hpp"
#include "private/renderer_type_conversions.hpp"
#include "renderer/buffers.hpp"
#include "renderer/context.hpp"
#include "renderer/frame.hpp"
#include "renderer/render_graph.hpp"
#include "renderer/renderer_image.hpp"
#include "renderer/test_model.hpp"
#include "resources/image.hpp"
#include "resources/model.hpp"
#include "scene/scene.hpp"

namespace mag
{
    namespace gfx
    {
        struct State
        {
                Context* context = nullptr;

                // Model data
                std::map<Model*, ref<VertexBuffer>> vertex_buffers;
                std::map<Model*, ref<IndexBuffer>> index_buffers;

                // Image data
                std::map<Image*, ref<RendererImage>> images;
        };

        static State* state = nullptr;

        b8 initialize(const Window& window)
        {
            state = new State();

            const ContextCreateOptions context_options = {.window = window};
            state->context = new Context(context_options);

            return state->context != nullptr;
        }

        void shutdown()
        {
            delete state->context;
            delete state;
        }

        void on_update(RenderGraph& render_graph, Scene& scene)
        {
            if (!state->context->begin_frame())
            {
                return;
            }

            state->context->begin_timestamp();  // Performance query

            render_graph.execute(scene);

            state->context->end_timestamp();

            // Present

            const auto& image = render_graph.get_output_attachment();
            const auto& extent = image.get_extent();

            state->context->end_frame(image, mag_to_vk(extent));
            state->context->calculate_timestamp();  // Calculate after command recording ended
        }

        void on_resize(const WindowResizeEvent& e);
        void on_event(const Event& e) { dispatch_event<WindowResizeEvent>(e, on_resize); }

        void draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex, const u32 first_instance)
        {
            auto& command_buffer = state->context->get_curr_frame().command_buffer;
            command_buffer.draw(vertex_count, instance_count, first_vertex, first_instance);
        }

        void draw_indexed(const u32 index_count, const u32 instance_count, const u32 first_index,
                          const i32 vertex_offset, const u32 first_instance)
        {
            auto& command_buffer = state->context->get_curr_frame().command_buffer;
            command_buffer.draw_indexed(index_count, instance_count, first_index, vertex_offset, first_instance);
        }

        void bind_buffers(Model* model)
        {
            auto vbo_it = state->vertex_buffers.find(model);
            auto ibo_it = state->index_buffers.find(model);

            if (vbo_it == state->vertex_buffers.end() || ibo_it == state->index_buffers.end())
            {
                LOG_ERROR("Model '{0}' was not uploaded to the GPU", static_cast<void*>(model));
                return;
            }

            auto& command_buffer = state->context->get_curr_frame().command_buffer;

            // Bind buffers
            command_buffer.bind_vertex_buffer(vbo_it->second->get_buffer());
            command_buffer.bind_index_buffer(ibo_it->second->get_buffer());
        }

        void bind_buffers(Line* line)
        {
            auto& command_buffer = state->context->get_curr_frame().command_buffer;
            command_buffer.bind_vertex_buffer(line->get_vbo().get_buffer());
        }

        ref<RendererImage> get_renderer_image(Image* image)
        {
            auto it = state->images.find(image);

            if (it == state->images.end())
            {
                LOG_ERROR("Image '{0}' was not uploaded to the GPU", static_cast<void*>(image));
                MAG_ASSERT(false, "@TODO: this shouldnt crash the application");
            }

            return it->second;
        }

        void upload_model(Model* model)
        {
            auto vbo_it = state->vertex_buffers.find(model);
            auto ibo_it = state->index_buffers.find(model);

            if (vbo_it != state->vertex_buffers.end() || ibo_it != state->index_buffers.end())
            {
                LOG_WARNING("Model '{0}' was already uploaded to the GPU", static_cast<void*>(model));
                return;
            }

            state->vertex_buffers[model] =
                create_ref<VertexBuffer>(model->vertices.data(), VEC_SIZE_BYTES(model->vertices));
            state->index_buffers[model] =
                create_ref<IndexBuffer>(model->indices.data(), VEC_SIZE_BYTES(model->indices));
        }

        void remove_model(Model* model)
        {
            auto vbo_it = state->vertex_buffers.find(model);
            auto ibo_it = state->index_buffers.find(model);

            if (vbo_it == state->vertex_buffers.end() || ibo_it == state->index_buffers.end())
            {
                LOG_ERROR("Tried to remove invalid model '{0}'", static_cast<void*>(model));
                return;
            }

            state->vertex_buffers.erase(vbo_it);
            state->index_buffers.erase(ibo_it);
        }

        void update_model(Model* model)
        {
            auto vbo_it = state->vertex_buffers.find(model);
            auto ibo_it = state->index_buffers.find(model);

            if (vbo_it == state->vertex_buffers.end() || ibo_it == state->index_buffers.end())
            {
                LOG_ERROR("Model '{0}' was not uploaded to the GPU", static_cast<void*>(model));
                return;
            }

            state->vertex_buffers[model]->resize(model->vertices.data(), VEC_SIZE_BYTES(model->vertices));
            state->index_buffers[model]->resize(model->indices.data(), VEC_SIZE_BYTES(model->indices));
        }

        ref<RendererImage> upload_image(Image* image, const SamplerAddressMode address_mode,
                                        const Filter min_mag_filter, const SamplerMipmapMode mip_map_mode)
        {
            auto it = state->images.find(image);

            if (it != state->images.end())
            {
                LOG_WARNING("Image '{0}' was already uploaded to the GPU", static_cast<void*>(image));
                return it->second;
            }

            // Use image channels to get color format
            ImageFormat image_format = ImageFormat::R8_Unorm;
            if (image->channels == 1)
            {
                image_format = ImageFormat::R8_Unorm;
            }

            else if (image->channels == 4)
            {
                image_format = ImageFormat::RGBA8_Srgb;
            }

            else
            {
                LOG_ERROR("Invalid texture format");
            }

            const uvec3 extent(image->width, image->height, 1);
            const vk::Format format = get_context().get_supported_color_format(image_format);

            state->images[image] =
                create_ref<RendererImage>(extent, ImageType::Texture, format,
                                          vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc |
                                              vk::ImageUsageFlagBits::eTransferDst,
                                          vk::ImageAspectFlagBits::eColor, min_mag_filter, address_mode, mip_map_mode,
                                          image->mip_levels, SampleCount::_1, image->pixels);

            return state->images[image];
        }

        void remove_image(Image* image)
        {
            auto it = state->images.find(image);

            if (it == state->images.end())
            {
                LOG_ERROR("Tried to remove invalid image '{0}'", static_cast<void*>(image));
                return;
            }

            state->images.erase(it);
        }

        void update_image(Image* image)
        {
            auto it = state->images.find(image);

            if (it == state->images.end())
            {
                LOG_ERROR("Image '{0}' was not uploaded to the GPU", static_cast<void*>(image));
                return;
            }

            it->second->set_pixels(image->pixels);
        }

        void on_resize(const WindowResizeEvent& e)
        {
            const uvec2& size = {e.width, e.height};

            state->context->get_device().waitIdle();

            state->context->recreate_swapchain(size);
        }
    };  // namespace gfx
};      // namespace mag
