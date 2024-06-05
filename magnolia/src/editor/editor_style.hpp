#pragma once

#include "imgui.h"

namespace mag
{
    inline void set_default_editor_style(ImGuiStyle &style)
    {
        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.07f;
        style.WindowRounding = 3.0f;
        style.WindowBorderSize = 0.0f;
        style.WindowMinSize = {150.0f, 100.0f};
        style.FrameRounding = 3.0f;
        style.FrameBorderSize = 0.0f;
        style.FramePadding = ImVec2(6.0f, 4.0f);
        style.SeparatorTextPadding.y = style.FramePadding.y;
        style.TabRounding = 1.0f;
        style.TabBorderSize = 0.0f;
        style.TabBarBorderSize = 0.0f;
        style.DockingSeparatorSize = 1.0f;

        const ImVec4 black_opaque = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        const ImVec4 white_opaque = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        const ImVec4 gray_opaque = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
        const ImVec4 blue_opaque = ImVec4(0.03f, 0.124f, 0.315f, 1.00f);

        style.Colors[ImGuiCol_Text] = white_opaque;
        // style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        // style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_WindowBg] = black_opaque;
        // style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
        style.Colors[ImGuiCol_Border] = gray_opaque;
        // style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, .00f, 1.00f, 1.0f);
        // style.Colors[ImGuiCol_FrameBg] = white_opaque;
        // style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        // style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_TitleBg] = gray_opaque;
        // style.Colors[ImGuiCol_TitleBgCollapsed] = white_opaque;
        style.Colors[ImGuiCol_TitleBgActive] = blue_opaque;
        style.Colors[ImGuiCol_MenuBarBg] = black_opaque;
        style.Colors[ImGuiCol_Tab] = gray_opaque;
        style.Colors[ImGuiCol_TabHovered] = blue_opaque;
        style.Colors[ImGuiCol_TabActive] = blue_opaque;
        style.Colors[ImGuiCol_TabUnfocused] = gray_opaque;
        style.Colors[ImGuiCol_TabUnfocusedActive] = gray_opaque;
        style.Colors[ImGuiCol_ScrollbarBg] = black_opaque;
        // style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
        // style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
        // style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
        // style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        // style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        // style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        // style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        // style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        // style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Header] = blue_opaque;
        // style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        // style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.926f, 0.059f, 0.098f, 1.00f);
        // style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
        // style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        // style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        // style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        // style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        // style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        // style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    }
};  // namespace mag
