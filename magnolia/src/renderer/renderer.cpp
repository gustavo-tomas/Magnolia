#include "renderer/renderer.hpp"

#include "core/assert.hpp"
#include "core/logger.hpp"
#include "renderer/buffers.hpp"
#include "resources/model.hpp"

namespace mag
{
    Renderer::Renderer(Window& window) : window(window)
    {
        // Create context
        const ContextCreateOptions context_options = {.window = window};
        context = create_unique<Context>(context_options);
        LOG_SUCCESS("Context initialized");
    }

    void Renderer::update(RenderGraph& render_graph)
    {
        this->context->begin_frame();
        this->context->begin_timestamp();  // Performance query

        render_graph.execute();

        context->end_timestamp();

        // Present

        const auto& image = render_graph.get_output_attachment();
        const auto& extent = image.get_extent();

        this->context->end_frame(image, extent);
        this->context->calculate_timestamp();  // Calculate after command recording ended
    }

    void Renderer::draw(const u32 vertex_count, const u32 instance_count, const u32 first_vertex,
                        const u32 first_instance)
    {
        auto& command_buffer = context->get_curr_frame().command_buffer;
        command_buffer.draw(vertex_count, instance_count, first_vertex, first_instance);
    }

    void Renderer::draw_indexed(const u32 index_count, const u32 instance_count, const u32 first_index,
                                const i32 vertex_offset, const u32 first_instance)
    {
        auto& command_buffer = context->get_curr_frame().command_buffer;
        command_buffer.draw_indexed(index_count, instance_count, first_index, vertex_offset, first_instance);
    }

    void Renderer::bind_buffers(Model* model)
    {
        auto vbo_it = vertex_buffers.find(model);
        auto ibo_it = index_buffers.find(model);

        if (vbo_it == vertex_buffers.end() || ibo_it == index_buffers.end())
        {
            LOG_ERROR("Model '{0}' was not uploaded to the GPU", static_cast<void*>(model));
            return;
        }

        auto& command_buffer = context->get_curr_frame().command_buffer;

        // Bind buffers
        command_buffer.bind_vertex_buffer(vbo_it->second->get_buffer());
        command_buffer.bind_index_buffer(ibo_it->second->get_buffer());
    }

    void Renderer::update_model(Model* model)
    {
        auto vbo_it = vertex_buffers.find(model);
        auto ibo_it = index_buffers.find(model);

        if (vbo_it == vertex_buffers.end() || ibo_it == index_buffers.end())
        {
            LOG_ERROR("Model '{0}' was not uploaded to the GPU", static_cast<void*>(model));
            return;
        }

        vertex_buffers[model]->resize(model->vertices.data(), VEC_SIZE_BYTES(model->vertices));
        index_buffers[model]->resize(model->indices.data(), VEC_SIZE_BYTES(model->indices));
    }

    void Renderer::upload_model(Model* model)
    {
        auto vbo_it = vertex_buffers.find(model);
        auto ibo_it = index_buffers.find(model);

        if (vbo_it != vertex_buffers.end() || ibo_it != index_buffers.end())
        {
            LOG_WARNING("Model '{0}' was already uploaded to the GPU", static_cast<void*>(model));
            return;
        }

        vertex_buffers[model] = create_ref<VertexBuffer>(model->vertices.data(), VEC_SIZE_BYTES(model->vertices));
        index_buffers[model] = create_ref<IndexBuffer>(model->indices.data(), VEC_SIZE_BYTES(model->indices));
    }

    void Renderer::remove_model(Model* model)
    {
        auto vbo_it = vertex_buffers.find(model);
        auto ibo_it = index_buffers.find(model);

        if (vbo_it == vertex_buffers.end() || ibo_it == index_buffers.end())
        {
            LOG_ERROR("Tried to remove invalid model '{0}'", static_cast<void*>(model));
            return;
        }

        vertex_buffers.erase(vbo_it);
        index_buffers.erase(ibo_it);
    }

    ref<RendererImage> Renderer::get_renderer_image(Image* image)
    {
        auto it = images.find(image);

        if (it == images.end())
        {
            LOG_ERROR("Image '{0}' was not uploaded to the GPU", static_cast<void*>(image));
            ASSERT(false, "@TODO: this shouldnt crash the application");
        }

        return it->second;
    }

    void Renderer::update_image(Image* image)
    {
        auto it = images.find(image);

        if (it == images.end())
        {
            LOG_ERROR("Image '{0}' was not uploaded to the GPU", static_cast<void*>(image));
            return;
        }

        it->second->set_pixels(image->pixels);
    }

    ref<RendererImage> Renderer::upload_image(Image* image)
    {
        auto it = images.find(image);

        if (it != images.end())
        {
            LOG_WARNING("Image '{0}' was already uploaded to the GPU", static_cast<void*>(image));
            return it->second;
        }

        const vk::Extent3D extent(image->width, image->height, 1);

        // @TODO: check for supported formats
        const vk::Format format = vk::Format::eR8G8B8A8Srgb;

        images[image] =
            create_ref<RendererImage>(extent, image->pixels, format,
                                      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc |
                                          vk::ImageUsageFlagBits::eTransferDst,
                                      vk::ImageAspectFlagBits::eColor, image->mip_levels, vk::SampleCountFlagBits::e1);

        return images[image];
    }

    void Renderer::remove_image(Image* image)
    {
        auto it = images.find(image);

        if (it == images.end())
        {
            LOG_ERROR("Tried to remove invalid image '{0}'", static_cast<void*>(image));
            return;
        }

        images.erase(it);
    }

    void Renderer::on_event(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowResizeEvent>(BIND_FN(Renderer::on_resize));
    }

    void Renderer::on_resize(WindowResizeEvent& e)
    {
        const uvec2& size = {e.width, e.height};

        context->get_device().waitIdle();

        context->recreate_swapchain(size);
    }
};  // namespace mag
