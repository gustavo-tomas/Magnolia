#include "magnolia/tools/console.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>

// This on top
#include <imgui/imgui.h>
//

#include <SDL3/SDL_properties.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_sdlrenderer3.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <map>
#include <numeric>
#include <utility>

#include "magnolia/core/assert.hpp"
#include "magnolia/core/logger.hpp"
#include "magnolia/core/string.hpp"
#include "magnolia/math/types.hpp"

// @TODO: we use the SDL renderer instead of vulkan for simplicity. Maybe its a
// good idea to reuse the window.cpp code to create windows and choose a backend.
// This would simplify input and event handling, but a lot of refactoring will
// be needed. For now we handle input events and updates in the window file.

namespace mag
{
#define COLOR_WHITE (math::vec4(1.0f, 1.0f, 1.0f, 1.0f))
#define COLOR_BLUE (math::vec4(0.42f, 0.64f, 0.89f, 1.0f))
#define COLOR_RED (math::vec4(1.0f, 0.2f, 0.2f, 1.0f))
#define COLOR_BROWN (math::vec4(1.0f, 0.8f, 0.6f, 1.0f))

    namespace console
    {
        struct LogData
        {
                math::vec4 color = COLOR_WHITE;
                str text;
        };

        struct ConsoleState
        {
                SDL_Window* window = nullptr;
                SDL_Renderer* renderer = nullptr;

                // We want this to be ordered
                std::map<str, std::function<void(const std::vector<str>&)>> commands;

                std::vector<LogData> items;
                std::vector<str> history;
                i64 history_pos = -1;  // -1: new line, 0..History.Size-1 browsing history.
                ImGuiTextFilter filter;
                u32 history_display_size = 15;
                b8 scroll_to_bottom = false;
                b8 auto_scroll = true;
        };

        static ConsoleState* state = nullptr;

        static void create_window();

        static void destroy_window();

        static void initialize_console();

        static void draw_console();

        static void scrolling_region(b8 copy_to_clipboard);

        static void handle_text_history(ImGuiInputTextCallbackData* data);

        static void handle_text_completion(ImGuiInputTextCallbackData* data);

        static i32 text_edit_callback(ImGuiInputTextCallbackData* data);

        static void clear_log();

        template <typename... Args>
        static void add_log(const LogData& log, const Args&... args);

        b8 initialize()
        {
            state = new ConsoleState();

            create_window();
            initialize_console();

            return state != nullptr;
        }

        void create_window()
        {
            const i32 width = 800;
            const i32 height = 600;
            const SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;

            state->window = SDL_CreateWindow("Console", width, height, flags);

            // @TODO: SDL3 renderer has memory leaks :( This is a workaround for fixing (most) of the leaks.
            // Update SDL3 submodule when issues get resolved
            // https://github.com/libsdl-org/SDL/issues/14973
            // https://github.com/libsdl-org/SDL/issues/15125

            const SDL_PropertiesID props = SDL_CreateProperties();
            MAG_ASSERT(SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, 1),
                       "Failed to set property: '{}'", SDL_GetError());

            MAG_ASSERT(SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, state->window),
                       "Failed to set property: '{}'", SDL_GetError());

            state->renderer = SDL_CreateRendererWithProperties(props);
            SDL_DestroyProperties(props);

            MAG_ASSERT(state->window != nullptr, "Failed to create SDL window: '{}'", SDL_GetError());

            MAG_ASSERT(state->renderer, "Failed to create SDL renderer: '{}'", SDL_GetError());

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
        }

        void initialize_console()
        {
            clear_log();

            // Mag commands are blue to distinguish from user commands

            register_command("HELP", [](const std::vector<str>&)
            {
                add_log({.color = COLOR_BLUE, .text = "Commands:"});
                for (const auto& [command, function] : state->commands)
                {
                    add_log({.color = COLOR_BLUE, .text = "- {0}"}, command.c_str());
                }
            });

            register_command("CLEAR", [](const std::vector<str>&) { clear_log(); });

            register_command("HISTORY", [](const std::vector<str>&)
            {
                const i64 first = static_cast<i64>(state->history.size()) - state->history_display_size;
                for (u64 i = first > 0 ? first : 0; i < state->history.size(); i++)
                {
                    add_log({.color = COLOR_BLUE, .text = "{0:3}: {1}"}, i, state->history[i].c_str());
                }
            });

            add_log({.text = "Command console. Enter 'HELP' for more information."});
        }

        void shutdown()
        {
            destroy_window();

            delete state;
        }

        void destroy_window()
        {
            ImGui_ImplSDLRenderer3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();

            SDL_DestroyRenderer(state->renderer);
            SDL_DestroyWindow(state->window);
        }

        void register_command(const str& command, const std::function<void(const std::vector<str>&)>&& func)
        {
            if (!state->commands.contains(command))
            {
                state->commands[command] = func;
                return;
            }

            LOG_WARNING("Command '{0}' is already registered", command);
        }

        void execute_command(const str& command, const std::vector<str>& args)
        {
            str full_command = command;

            // Include paths
            full_command = std::accumulate(args.begin(), args.end(), full_command,
                                           [](const str& cmd, const str& arg) { return cmd + " " + arg; });

            add_log({.color = COLOR_BROWN, .text = "# {0}"}, full_command);

            // Insert into state->history
            state->history_pos = -1;
            state->history.push_back(full_command);

            // Process command
            auto it = state->commands.find(command);
            if (it == state->commands.end())
            {
                add_log({.color = COLOR_RED, .text = "Unknown command: '{0}'"}, command);
            }

            else
            {
                it->second(args);
            }

            // On command input, we scroll to bottom even if AutoScroll == false
            state->scroll_to_bottom = true;
        }

        void on_update()
        {
            const ImGuiIO& io = ImGui::GetIO();
            (void)io;

            SDL_Window* window = state->window;
            SDL_Renderer* renderer = state->renderer;

            if ((SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) != 0U)
            {
                return;
            }

            // Frame start
            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            const ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode |
                                                  ImGuiDockNodeFlags_NoUndocking | ImGuiDockNodeFlags_AutoHideTabBar;

            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dock_flags);

            draw_console();

            const math::vec4 draw_color = {0.4, 0.4, 0.4, 1.0};

            // Track previous scale to avoid redundant calls
            static math::vec2 prev_render_scale = {0, 0};

            if (io.DisplayFramebufferScale.x != prev_render_scale.x ||
                io.DisplayFramebufferScale.y != prev_render_scale.y)
            {
                SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
                prev_render_scale.x = io.DisplayFramebufferScale.x;
                prev_render_scale.y = io.DisplayFramebufferScale.y;
            }

            // Frame end
            ImGui::Render();
            SDL_SetRenderDrawColorFloat(renderer, draw_color.r, draw_color.g, draw_color.b, draw_color.a);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
            SDL_RenderPresent(renderer);
        }

        void scrolling_region(const b8 copy_to_clipboard)
        {
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("Clear"))
                {
                    clear_log();
                }
                ImGui::EndPopup();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));  // Tighten spacing
            if (copy_to_clipboard)
            {
                ImGui::LogToClipboard();
            }

            for (const LogData& item : state->items)
            {
                const c8* text = item.text.c_str();

                if (!state->filter.PassFilter(text))
                {
                    continue;
                }

                const ImVec4 color = ImVec4(item.color.r, item.color.g, item.color.b, item.color.a);

                ImGui::PushStyleColor(ImGuiCol_Text, color);

                ImGui::TextUnformatted(text);

                ImGui::PopStyleColor();
            }

            if (copy_to_clipboard)
            {
                ImGui::LogFinish();
            }

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning
            // of the frame. Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (state->scroll_to_bottom || (state->auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            {
                ImGui::SetScrollHereY(1.0F);
            }

            state->scroll_to_bottom = false;

            ImGui::PopStyleVar();
        }

        void draw_console()
        {
            if (!ImGui::Begin("Console"))
            {
                ImGui::End();
                return;
            }

            ImGui::SameLine();
            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_L, ImGuiInputFlags_Tooltip);
            if (ImGui::SmallButton("Clear"))
            {
                clear_log();
            }

            ImGui::SameLine();
            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_C, ImGuiInputFlags_Tooltip);
            const b8 copy_to_clipboard = ImGui::SmallButton("Copy");

            ImGui::Separator();

            // Options menu
            if (ImGui::BeginPopup("Options"))
            {
                ImGui::Checkbox("Auto-scroll", &state->auto_scroll);
                ImGui::EndPopup();
            }

            // Options, Filter
            if (ImGui::Button("Options"))
            {
                ImGui::OpenPopup("Options");
            }

            const u32 line_width = 180;
            ImGui::SameLine();
            state->filter.Draw(R"(Filter ("incl,-excl") ("error"))", line_width);

            ImGui::Separator();

            // Reserve enough left-over height for 1 separator + 1 input text
            const f32 footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

            if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened,
                                  ImGuiWindowFlags_HorizontalScrollbar))
            {
                scrolling_region(copy_to_clipboard);
            }

            ImGui::EndChild();

            ImGui::Separator();

            // Command-line
            b8 reclaim_focus = false;

            const ImGuiInputTextFlags input_text_flags =
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll |
                ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;

            str command_line;

            ImGui::PushItemWidth(-1);

            if (ImGui::InputText("##Input", &command_line, input_text_flags,
                                 [](ImGuiInputTextCallbackData* data) -> i32 { return text_edit_callback(data); }))
            {
                reclaim_focus = true;

                string::trim(command_line);

                std::vector<str> substrings;

                // Separate command from args
                string::split(command_line, " ", substrings);

                if (!substrings.empty())
                {
                    const str& command = substrings.front();
                    const std::vector<str> args(substrings.begin() + 1, substrings.end());

                    execute_command(command, args);
                }
            }

            ImGui::PopItemWidth();

            // Auto-focus on window apparition
            ImGui::SetItemDefaultFocus();

            // Auto focus previous widget
            if (reclaim_focus)
            {
                ImGui::SetKeyboardFocusHere(-1);
            }

            ImGui::End();
        }

        void handle_text_history(ImGuiInputTextCallbackData* data)
        {
            const i64 prev_history_pos = state->history_pos;

            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (state->history_pos == -1)
                {
                    state->history_pos = static_cast<i64>(state->history.size()) - 1;
                }
                else if (state->history_pos > 0)
                {
                    state->history_pos--;
                }
            }

            else if (data->EventKey == ImGuiKey_DownArrow && state->history_pos != -1)
            {
                if (std::cmp_greater_equal(++state->history_pos, state->history.size()))
                {
                    state->history_pos = -1;
                }
            }

            // A better implementation would preserve the data on the current input line along with
            // cursor position.
            if (prev_history_pos != state->history_pos)
            {
                const str history_str = (state->history_pos >= 0) ? state->history[state->history_pos] : "";

                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str.c_str());
            }
        }

        void handle_text_completion(ImGuiInputTextCallbackData* data)
        {
            // Locate beginning of current word
            const c8* word_end = data->Buf + data->CursorPos;
            const c8* word_start = word_end;
            while (word_start > data->Buf)
            {
                const c8 c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                {
                    break;
                }
                word_start--;
            }

            // Build a list of candidates
            std::vector<str> candidates;

            for (const auto& [command, function] : state->commands)
            {
                const b8 equal = string::equalni(command, word_start, static_cast<i32>(word_end - word_start));

                if (equal)
                {
                    candidates.push_back(command);
                }
            }

            // No match
            if (candidates.empty())
            {
                add_log({.color = COLOR_RED, .text = "No match for '{0}'!"}, word_start);
                return;
            }

            // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
            if (candidates.size() == 1)
            {
                data->DeleteChars(static_cast<i32>(word_start - data->Buf), static_cast<i32>(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0].c_str());
                data->InsertChars(data->CursorPos, " ");
                return;
            }

            // Multiple matches. Complete as much as we can.
            i32 match_len = static_cast<i32>(word_end - word_start);
            b8 all_candidates_matches = true;

            while (all_candidates_matches)
            {
                i32 c = 0;

                for (u64 i = 0; i < candidates.size() && all_candidates_matches; i++)
                {
                    const i32 candidate_c = std::toupper(candidates[i][match_len]);
                    if (i == 0)
                    {
                        c = candidate_c;
                    }
                    else if (c == 0 || c != candidate_c)
                    {
                        all_candidates_matches = false;
                    }
                }

                if (all_candidates_matches)
                {
                    match_len++;
                }
            }

            if (match_len > 0)
            {
                data->DeleteChars(static_cast<i32>(word_start - data->Buf), static_cast<i32>(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0].c_str(), candidates[0].c_str() + match_len);
            }

            // List matches
            add_log({.text = "Possible matches:"});

            for (const str& candidate : candidates)
            {
                add_log({.text = "- {0}"}, candidate);
            }
        }

        i32 text_edit_callback(ImGuiInputTextCallbackData* data)
        {
            switch (data->EventFlag)
            {
                // Text completion
                case ImGuiInputTextFlags_CallbackCompletion:
                {
                    handle_text_completion(data);
                    break;
                }

                // History
                case ImGuiInputTextFlags_CallbackHistory:
                {
                    handle_text_history(data);
                    break;
                }

                default:
                    break;
            }

            return 0;
        }

        template <typename... Args>
        void add_log(const LogData& log, const Args&... args)
        {
            const str formatted_text = log::get_formatted_log(log.text, args...);

            state->items.push_back({.color = log.color, .text = formatted_text});
        }

        void clear_log() { state->items.clear(); }

        void on_event(const void* event)
        {
            const auto* e = static_cast<const SDL_Event*>(event);
            ImGui_ImplSDL3_ProcessEvent(e);
        }

        u32 get_window_id() { return SDL_GetWindowID(state->window); }
    };  // namespace console
};  // namespace mag
