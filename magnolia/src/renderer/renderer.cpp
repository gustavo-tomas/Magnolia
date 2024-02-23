#include "renderer/renderer.hpp"

#include "core/logger.hpp"

namespace mag
{
    void Renderer::initialize(Window& window)
    {
        this->window = std::addressof(window);

        // Create context
        ContextCreateOptions context_options = {.window = window};
        context_options.application_name = "Magnolia";
        context_options.engine_name = "Magnolia";

        this->context.initialize(context_options);
        LOG_SUCCESS("Context initialized");

        this->render_pass.initialize({context.get_surface_extent().width, context.get_surface_extent().height});
        LOG_SUCCESS("RenderPass initialized");

        camera.initialize({10.35f, 5.13f, 10.35f}, {-20.0f, -45.0f, 0.0f}, 60.0f, window.get_size(), 0.1f, 1000.0f);
        LOG_SUCCESS("Camera initialized");

        controller.initialize(&camera, &window);
        LOG_SUCCESS("Controller initialized");

        render_pass.set_camera(&camera);

        // Create a cube mesh
        mesh.vertices.resize(8);

        mesh.vertices[0].position = {-1.0f, -1.0f, 1.0f};
        mesh.vertices[1].position = {1.0f, -1.0f, 1.0f};
        mesh.vertices[2].position = {1.0f, 1.0f, 1.0f};
        mesh.vertices[3].position = {-1.0f, 1.0f, 1.0f};

        mesh.vertices[4].position = {-1.0f, -1.0f, -1.0f};
        mesh.vertices[5].position = {1.0f, -1.0f, -1.0f};
        mesh.vertices[6].position = {1.0f, 1.0f, -1.0f};
        mesh.vertices[7].position = {-1.0f, 1.0f, -1.0f};

        mesh.vertices[0].normal = normalize(mesh.vertices[0].position);
        mesh.vertices[1].normal = normalize(mesh.vertices[1].position);
        mesh.vertices[2].normal = normalize(mesh.vertices[2].position);
        mesh.vertices[3].normal = normalize(mesh.vertices[3].position);

        mesh.vertices[4].normal = normalize(mesh.vertices[4].position);
        mesh.vertices[5].normal = normalize(mesh.vertices[5].position);
        mesh.vertices[6].normal = normalize(mesh.vertices[6].position);
        mesh.vertices[7].normal = normalize(mesh.vertices[7].position);

        mesh.indices = {// Front face
                        0, 1, 2, 2, 3, 0,
                        // Back face
                        4, 5, 6, 6, 7, 4,
                        // Left face
                        0, 3, 7, 7, 4, 0,
                        // Right face
                        1, 2, 6, 6, 5, 1,
                        // Top face
                        3, 2, 6, 6, 7, 3,
                        // Bottom face
                        0, 1, 5, 5, 4, 0};

        mesh.vbo.initialize(mesh.vertices.data(), VECSIZE(mesh.vertices) * sizeof(Vertex), context.get_allocator());
        mesh.ibo.initialize(mesh.indices.data(), VECSIZE(mesh.indices) * sizeof(u32), context.get_allocator());
    }

    void Renderer::shutdown()
    {
        this->context.get_device().waitIdle();

        mesh.vbo.shutdown();
        mesh.ibo.shutdown();

        this->controller.shutdown();
        LOG_SUCCESS("Controller destroyed");

        this->camera.shutdown();
        LOG_SUCCESS("Camera destroyed");

        this->render_pass.shutdown();
        LOG_SUCCESS("RenderPass destroyed");

        this->context.shutdown();
        LOG_SUCCESS("Context destroyed");
    }

    void Renderer::update(Editor& editor, const f32 dt)
    {
        // @TODO: maybe this shouldnt be here
        controller.update(dt);

        // Skip rendering if minimized
        if (window->is_minimized()) return;

        Frame& curr_frame = context.get_curr_frame();
        Pass& pass = render_pass.get_pass();

        this->context.begin_frame();

        // @TODO: testing
        if (window->is_key_down(SDLK_UP))
            render_pass.set_render_scale(render_pass.get_render_scale() + 0.01f);

        else if (window->is_key_down(SDLK_DOWN))
            render_pass.set_render_scale(render_pass.get_render_scale() - 0.01f);
        // @TODO: testing

        // Draw calls
        render_pass.before_render(curr_frame.command_buffer);
        curr_frame.command_buffer.begin_rendering(pass);
        render_pass.render(curr_frame.command_buffer, mesh);
        curr_frame.command_buffer.end_rendering();
        render_pass.after_render(curr_frame.command_buffer);

        // @TODO: maybe dont do this here
        editor.update(curr_frame.command_buffer, render_pass.get_draw_image());

        // Present

        // @TODO: testing
        static bool swap = false;
        if (window->is_key_pressed(SDLK_LSHIFT)) swap = !swap;

        if (swap)
        {
            const auto extent = render_pass.get_draw_size();
            this->context.end_frame(render_pass.get_draw_image(), {extent.x, extent.y, extent.z});
        }

        else
        {
            const auto extent = editor.get_draw_size();
            this->context.end_frame(editor.get_image(), {extent.x, extent.y, 1});
        }
        // @TODO: testing
    }

    void Renderer::on_resize(const uvec2& size)
    {
        context.get_device().waitIdle();

        // Use the surface extent after recreating the swapchain
        context.recreate_swapchain(size, vk::PresentModeKHR::eImmediate);
        const uvec2 surface_extent = uvec2(context.get_surface_extent().width, context.get_surface_extent().height);

        this->render_pass.on_resize(surface_extent);
        this->camera.set_aspect_ratio(surface_extent);
    }

    void Renderer::on_mouse_move(const ivec2& mouse_dir) { this->controller.on_mouse_move(mouse_dir); }
};  // namespace mag
