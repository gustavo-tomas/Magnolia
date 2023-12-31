#include "core/window.hpp"

#include "core/logger.hpp"

namespace mag
{
    void Window::initialize(const WindowOptions& options)
    {
        ASSERT(SDL_Init(SDL_INIT_VIDEO) == 0, "Failed to initialize SDL: " + str(SDL_GetError()));

        const u32 flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

        handle = SDL_CreateWindow(options.title.c_str(), options.position.x, options.position.y, options.size.x,
                                  options.size.y, flags);

        ASSERT(handle != nullptr, "Failed to create SDL window" + str(SDL_GetError()));

        u32 count = 0;
        ASSERT(SDL_Vulkan_GetInstanceExtensions(this->handle, &count, nullptr),
               "Failed to enumerate window extensions");

        extensions.resize(count);
        ASSERT(SDL_Vulkan_GetInstanceExtensions(this->handle, &count, extensions.data()), "Failed to get extensions");
    }

    void Window::shutdown()
    {
        SDL_DestroyWindow(handle);
        SDL_Quit();
    }

    b8 Window::update()
    {
        update_counter++;

        SDL_Event e;

        while (SDL_PollEvent(&e) != 0)
        {
            const SDL_Keycode key = e.key.keysym.sym;

            switch (e.type)
            {
                case SDL_QUIT:
                    return false;
                    break;

                case SDL_KEYDOWN:
                    this->key_press(key);

                    if (e.key.repeat == 1) continue;
                    key_state[key] = true;
                    key_update[key] = update_counter;
                    break;

                case SDL_KEYUP:
                    this->key_release(key);

                    key_state[key] = false;
                    key_update[key] = update_counter;
                    break;

                case SDL_MOUSEMOTION:
                    // Ignore first mouse move after capturing cursor
                    if (!ignore_mouse_motion_events) mouse_move(ivec2(e.motion.xrel, e.motion.yrel));
                    ignore_mouse_motion_events = false;
                    break;

                case SDL_WINDOWEVENT:
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                        this->resize(uvec2(e.window.data1, e.window.data2));
                    break;
            }
        }

        return true;
    }

    vk::SurfaceKHR Window::create_surface(const vk::Instance instance) const
    {
        VkSurfaceKHR surface = 0;
        ASSERT(SDL_Vulkan_CreateSurface(handle, instance, &surface), "Failed to create surface");

        return surface;
    }

    void Window::on_resize(std::function<void(const uvec2&)> callback) { this->resize = std::move(callback); }

    void Window::on_key_press(std::function<void(const SDL_Keycode key)> callback)
    {
        this->key_press = std::move(callback);
    }

    void Window::on_key_release(std::function<void(const SDL_Keycode key)> callback)
    {
        this->key_release = std::move(callback);
    }

    void Window::on_mouse_move(std::function<void(const ivec2&)> callback) { this->mouse_move = std::move(callback); }

    b8 Window::is_key_pressed(const SDL_Keycode key) { return key_state[key] && (key_update[key] == update_counter); }

    b8 Window::is_key_down(const SDL_Keycode key) { return key_state[key]; }

    b8 Window::is_mouse_captured() const { return static_cast<b8>(SDL_GetRelativeMouseMode()); }

    b8 Window::is_minimized() const
    {
        const auto size = this->get_size();
        const auto flags = SDL_GetWindowFlags(this->handle);
        return (flags & SDL_WINDOW_MINIMIZED) || (size.x < 100 || size.y < 100);

        // Might be worth checking these too: || !(flags & SDL_WINDOW_INPUT_FOCUS) || !(flags & SDL_WINDOW_MOUSE_FOCUS)
    }

    void Window::set_capture_mouse(b8 capture)
    {
        // Oh SDL...
        if (SDL_SetRelativeMouseMode(static_cast<SDL_bool>(capture)) != 0) LOG_ERROR("Failed to set mouse mode");
        ignore_mouse_motion_events = true;
    }

    void Window::set_title(const str& title) { SDL_SetWindowTitle(handle, title.c_str()); }

    void Window::set_resizable(const b8 resizable)
    {
        SDL_SetWindowResizable(this->handle, static_cast<SDL_bool>(resizable));
    }

    ivec2 Window::get_mouse_position() const
    {
        ivec2 mouse_pos;
        SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        return mouse_pos;
    }

    uvec2 Window::get_size() const
    {
        uvec2 size;
        SDL_Vulkan_GetDrawableSize(handle, reinterpret_cast<i32*>(&size.x), reinterpret_cast<i32*>(&size.y));
        return size;
    }
};  // namespace mag
