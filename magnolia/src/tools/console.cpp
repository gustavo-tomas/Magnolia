#include "magnolia/tools/console.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

// This on top
#include <imgui/imgui.h>
//

#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_sdlrenderer3.h>

#include <unordered_map>

#include "magnolia/core/assert.hpp"
#include "magnolia/core/logger.hpp"

// @TODO: we use the SDL renderer instead of vulkan for simplicity. Maybe its a
// good idea to reuse the window.cpp code to create windows and choose a backend.
// This would simplify input and event handling, but a lot of refactoring will
// be needed. For now we handle input events and updates in the window file.

namespace mag
{
    namespace console
    {
        struct ConsoleState
        {
                SDL_Window* window = nullptr;
                SDL_Renderer* renderer = nullptr;

                std::unordered_map<str, std::function<void(const str&)>> commands;
        };

        static ConsoleState* state = nullptr;

        b8 initialize()
        {
            state = new ConsoleState();

            const i32 width = 800;
            const i32 height = 600;
            const SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;

            state->window = SDL_CreateWindow("Console", width, height, flags);
            state->renderer = SDL_CreateRenderer(state->window, nullptr);

            MAG_ASSERT(state->window != nullptr, "Failed to create SDL window: " + str(SDL_GetError()));

            MAG_ASSERT(state->renderer, "Failed to create SDL renderer: " + str(SDL_GetError()));

            SDL_SetWindowPosition(state->window, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED);

            // @TODO
            // set_window_icon(window_icon);

            SDL_ShowWindow(state->window);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO();
            (void)io;

            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

            ImGui::StyleColorsDark();

            // Setup scaling
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(1);
            style.FontScaleDpi = 1;

            ImGui_ImplSDL3_InitForSDLRenderer(state->window, state->renderer);
            ImGui_ImplSDLRenderer3_Init(state->renderer);

            return state != nullptr;
        }

        void shutdown()
        {
            ImGui_ImplSDLRenderer3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();

            SDL_DestroyRenderer(state->renderer);
            SDL_DestroyWindow(state->window);

            delete state;
        }

        void register_command(const str& command, const std::function<void(const str args)>& func)
        {
            if (!state->commands.contains(command))
            {
                state->commands[command] = std::move(func);
                return;
            }

            LOG_WARNING("Command '{0}' is already registered", command);
        }

        void execute_command(const str& command, const str& args)
        {
            auto it = state->commands.find(command);
            if (it != state->commands.end())
            {
                it->second(args);
                return;
            }

            LOG_ERROR("Command '{0}' is not registered", command);
        }

        void on_update()
        {
            ImGuiIO& io = ImGui::GetIO();
            (void)io;

            SDL_Window* window = state->window;
            SDL_Renderer* renderer = state->renderer;

            if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
            {
                SDL_Delay(10);
                return;
            }

            // Frame start
            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            const ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode |
                                                  ImGuiDockNodeFlags_NoUndocking | ImGuiDockNodeFlags_AutoHideTabBar;

            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dock_flags);

            ImGui::ShowDemoWindow();

            // Frame end
            ImGui::Render();
            SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
            SDL_SetRenderDrawColorFloat(renderer, 0.4, 0.4, 0.4, 1.0);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
            SDL_RenderPresent(renderer);
        }

        void on_event(const void* event)
        {
            const SDL_Event* e = static_cast<const SDL_Event*>(event);
            ImGui_ImplSDL3_ProcessEvent(e);
        }

        u32 get_window_id() { return SDL_GetWindowID(state->window); }
    };  // namespace console
};      // namespace mag
