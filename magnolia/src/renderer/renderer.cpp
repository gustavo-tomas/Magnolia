#include "renderer/renderer.hpp"

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

namespace mag
{
    struct Renderer::IMPL
    {
            IMPL() = default;
            ~IMPL() = default;

            unique<Context> context;

            // Model data
            std::map<Model*, ref<VertexBuffer>> vertex_buffers;
            std::map<Model*, ref<IndexBuffer>> index_buffers;

            // Image data
            std::map<Image*, ref<RendererImage>> images;
    };

    Renderer::Renderer(Window& window) : impl(new IMPL())
    {
        // Create context
        const ContextCreateOptions context_options = {.window = window};
        impl->context = create_unique<Context>(context_options);
        LOG_SUCCESS("Context initialized");
    }

    Renderer::~Renderer() = default;

    void Renderer::on_update(RenderGraph& render_graph)
    {
        if (!impl->context->begin_frame()) return;
        impl->context->begin_timestamp();  // Performance query

        render_graph.execute();

        impl->context->end_timestamp();

        // Present

        const auto& image = render_graph.get_output_attachment();
        const auto& extent = image.get_extent();

        impl->context->end_frame(image, mag_to_vk(extent));
        impl->context->calculate_timestamp();  // Calculate after command recording ended
    }

    void Renderer::draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex,
                        const u32 first_instance)
    {
        auto& command_buffer = impl->context->get_curr_frame().command_buffer;
        command_buffer.draw(vertex_count, instance_count, first_vertex, first_instance);
    }

    void Renderer::draw_indexed(const u32 index_count, const u32 instance_count, const u32 first_index,
                                const i32 vertex_offset, const u32 first_instance)
    {
        auto& command_buffer = impl->context->get_curr_frame().command_buffer;
        command_buffer.draw_indexed(index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void Renderer::bind_buffers(Model* model)
    {
        auto vbo_it = impl->vertex_buffers.find(model);
        auto ibo_it = impl->index_buffers.find(model);

        if (vbo_it == impl->vertex_buffers.end() || ibo_it == impl->index_buffers.end())
        {
            LOG_ERROR("Model '{0}' was not uploaded to the GPU", static_cast<void*>(model));
            return;
        }

        auto& command_buffer = impl->context->get_curr_frame().command_buffer;

        // Bind buffers
        command_buffer.bind_vertex_buffer(vbo_it->second->get_buffer());
        command_buffer.bind_index_buffer(ibo_it->second->get_buffer());
    }

    void Renderer::bind_buffers(Line* line)
    {
        auto& command_buffer = impl->context->get_curr_frame().command_buffer;

        command_buffer.bind_vertex_buffer(line->get_vbo().get_buffer());
    }

    void Renderer::update_model(Model* model)
    {
        auto vbo_it = impl->vertex_buffers.find(model);
        auto ibo_it = impl->index_buffers.find(model);

        if (vbo_it == impl->vertex_buffers.end() || ibo_it == impl->index_buffers.end())
        {
            LOG_ERROR("Model '{0}' was not uploaded to the GPU", static_cast<void*>(model));
            return;
        }

        impl->vertex_buffers[model]->resize(model->vertices.data(), VEC_SIZE_BYTES(model->vertices));
        impl->index_buffers[model]->resize(model->indices.data(), VEC_SIZE_BYTES(model->indices));
    }

    void Renderer::upload_model(Model* model)
    {
        auto vbo_it = impl->vertex_buffers.find(model);
        auto ibo_it = impl->index_buffers.find(model);

        if (vbo_it != impl->vertex_buffers.end() || ibo_it != impl->index_buffers.end())
        {
            LOG_WARNING("Model '{0}' was already uploaded to the GPU", static_cast<void*>(model));
            return;
        }

        impl->vertex_buffers[model] = create_ref<VertexBuffer>(model->vertices.data(), VEC_SIZE_BYTES(model->vertices));
        impl->index_buffers[model] = create_ref<IndexBuffer>(model->indices.data(), VEC_SIZE_BYTES(model->indices));
    }

    void Renderer::remove_model(Model* model)
    {
        auto vbo_it = impl->vertex_buffers.find(model);
        auto ibo_it = impl->index_buffers.find(model);

        if (vbo_it == impl->vertex_buffers.end() || ibo_it == impl->index_buffers.end())
        {
            LOG_ERROR("Tried to remove invalid model '{0}'", static_cast<void*>(model));
            return;
        }

        impl->vertex_buffers.erase(vbo_it);
        impl->index_buffers.erase(ibo_it);
    }

    ref<RendererImage> Renderer::get_renderer_image(Image* image)
    {
        auto it = impl->images.find(image);

        if (it == impl->images.end())
        {
            LOG_ERROR("Image '{0}' was not uploaded to the GPU", static_cast<void*>(image));
            ASSERT(false, "@TODO: this shouldnt crash the application");
        }

        return it->second;
    }

    void Renderer::update_image(Image* image)
    {
        auto it = impl->images.find(image);

        if (it == impl->images.end())
        {
            LOG_ERROR("Image '{0}' was not uploaded to the GPU", static_cast<void*>(image));
            return;
        }

        it->second->set_pixels(image->pixels);
    }

    ref<RendererImage> Renderer::upload_image(Image* image)
    {
        auto it = impl->images.find(image);

        if (it != impl->images.end())
        {
            LOG_WARNING("Image '{0}' was already uploaded to the GPU", static_cast<void*>(image));
            return it->second;
        }

        const uvec3 extent(image->width, image->height, 1);
        const vk::Format format = get_context().get_supported_color_format(ImageFormat::Srgb);

        impl->images[image] =
            create_ref<RendererImage>(extent, ImageType::Texture, image->pixels, format,
                                      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc |
                                          vk::ImageUsageFlagBits::eTransferDst,
                                      vk::ImageAspectFlagBits::eColor, image->mip_levels, SampleCount::_1);

        return impl->images[image];
    }

    void Renderer::remove_image(Image* image)
    {
        auto it = impl->images.find(image);

        if (it == impl->images.end())
        {
            LOG_ERROR("Tried to remove invalid image '{0}'", static_cast<void*>(image));
            return;
        }

        impl->images.erase(it);
    }

    void Renderer::on_event(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowResizeEvent>(BIND_FN(Renderer::on_resize));
    }

    void Renderer::on_resize(WindowResizeEvent& e)
    {
        const uvec2& size = {e.width, e.height};

        impl->context->get_device().waitIdle();

        impl->context->recreate_swapchain(size);
    }
};  // namespace mag
